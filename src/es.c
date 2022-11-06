#include "es.h"
#include <Rembedded.h>
#include <Rinternals.h>
#include <event.h>
#include <stdio.h>
#include <string.h>

void sighandler(int signal, short events, void *base) {
  event_base_loopbreak(base);
}

void source(const char *operation, const char *name) {
  SEXP e;

  PROTECT(e = lang2(install(operation), mkString(name)));
  R_tryEval(e, R_GlobalEnv, NULL);
  UNPROTECT(1);
}

int gender_checker(const char *gender) {
  const char male[] = "male";
  const char female[] = "female";
  if (memcmp(male, gender, sizeof(male)) &&
      memcmp(female, gender, sizeof(female))) {
    return ARG_NOT_ACCEPTABLE;
  }
  return NO_ERROR;
}

int all_pass_checker() { return NO_ERROR; }

void req_cb(__SG_UNUSED void *cls, struct sg_httpreq *req,
            struct sg_httpres *res) {
  double score = 0;
  struct sg_str *payload = sg_httpreq_payload(req);
  const char *json_string = sg_str_content(payload);
  struct json_object *json_jo = json_tokener_parse(json_string);
  struct json_object *rsp_jo = json_object_new_object();

  if (!json_jo) {
    json_object_object_add(rsp_jo, "ERROR",
                           json_object_new_string("There is no payload"));
    goto SEND_RESPONSE;
  }

  struct json_object *function_jo = json_object_object_get(json_jo, "function");
  if (!function_jo) {
    json_object_object_add(
        rsp_jo, "ERROR", json_object_new_string("There is no key [function]"));
    goto SEND_RESPONSE;
  }
  const char *function = json_object_get_string(function_jo);
  int function_len = json_object_get_string_len(function_jo);
  int index = (sizeof(es_funs) / sizeof(struct es_function));
  int errorOccurred = 0;
  while (index--) {
    if (function_len == es_funs[index].length &&
        !memcmp(es_funs[index].name, function, function_len)) {
      errorOccurred = es_funs[index].function(function, json_jo, &score);
      break;
    }
  }
  if (index < 0) {
    json_object_object_add(
        rsp_jo, "ERROR", json_object_new_string("There is no key [function]"));
    goto SEND_RESPONSE;
  }

  if (!errorOccurred) {
    json_object_object_add(rsp_jo, "score", json_object_new_double(score));
  } else {
    json_object_object_add(rsp_jo, "ERROR",
                           json_object_new_string("process failed!"));
  }

SEND_RESPONSE:
  sg_httpres_send(res, json_object_to_json_string(rsp_jo), "application/json",
                  200);

  json_object_put(rsp_jo);
  json_object_put(json_jo);
}

int reach_score(const char *function, struct json_object *json_jo,
                double *score) {
  if (!function || !json_jo) {
    return ARG_NOT_ACCEPTABLE;
  }

  struct json_object *args_jo = json_object_object_get(json_jo, "arguments");
  if (!args_jo) {
    return ARG_NOT_ACCEPTABLE;
  }

  SEXP fun = Rf_install(function);

  SEXP call =
      PROTECT(Rf_allocVector(LANGSXP, json_object_object_length(args_jo) + 1));
  SETCAR(call, fun);

  SEXP s = CDR(call);
  int index = (sizeof(rsas) / sizeof(struct es_arg));
  while (index--) {
    struct json_object *tmp_jo =
        json_object_object_get(args_jo, rsas[index].name);
    SEXP tmp = NULL;
    if (tmp_jo) {
      switch (json_object_get_type(tmp_jo)) {
      case json_type_boolean:
        int bool_value = json_object_get_boolean(tmp_jo);
        if (rsas[index].type == BOOL &&
            rsas[index].checker(bool_value) == NO_ERROR) {
          tmp = rsas[index].converter(bool_value);
        } else {
          printf("key:[%s] value:[%d] is not acceptable\n", rsas[index].name,
                 bool_value);
        }
        break;
      case json_type_double:
        double double_value = json_object_get_double(tmp_jo);
        if (rsas[index].type == DOUBLE &&
            rsas[index].checker(double_value) == NO_ERROR) {
          tmp = rsas[index].converter(double_value);
        } else {
          printf("key:[%s] value:[%f] is not acceptable\n", rsas[index].name,
                 double_value);
        }
        break;
      case json_type_int:
        int int_value = json_object_get_int(tmp_jo);
        if (rsas[index].type == INT &&
            rsas[index].checker(int_value) == NO_ERROR) {
          tmp = rsas[index].converter(int_value);
        } else {
          printf("key:[%s] value:[%d] is not acceptable\n", rsas[index].name,
                 int_value);
        }
        break;
      case json_type_string:
        const char *string_value = json_object_get_string(tmp_jo);
        if (rsas[index].type == STRING &&
            rsas[index].checker(string_value) == NO_ERROR) {
          tmp = rsas[index].converter(string_value);
        } else {
          printf("key:[%s] value:[%s] is not acceptable\n", rsas[index].name,
                 string_value);
        }
        break;
      }
      SETCAR(s, tmp);
      SET_TAG(s, Rf_install(rsas[index].name));
      if (index != 0) {
        s = CDR(s);
      }
    }
  }

  // Execute the function
  int errorOccurred = 0;
  SEXP ret = R_tryEvalSilent(call, R_GlobalEnv, &errorOccurred);

  if (!errorOccurred) {
    *score = *REAL(ret);
    printf("R returned: [%lf]\n", *score);
  } else {
    printf("Error occurred calling R\n");
  }

  R_ReleaseObject(ret);
  R_ReleaseObject(call);
  R_ReleaseObject(fun);

  UNPROTECT(1);

  return errorOccurred;
}
