#ifndef ES_H
#define ES_H

#include <Rinternals.h>
#include <event.h>
#include <json-c/json.h>
#include <sagui/sagui.h>

/**
 * @brief error types
 */
enum error_type { NO_ERROR, ARG_NOT_ACCEPTABLE };

/**
 * @brief argument date types
 */
enum data_type { INT, DOUBLE, BOOL, STRING };

/**
 * @brief check gender is acceptable
 * @param gender only male and female are acceptable
 */
int gender_checker(const char *gender);

/**
 * @brief check nothing
 */
int all_pass_checker();

/**
 * @brief API Supported function object
 */
struct es_function {
  char *name; /**< function name from REST API */
  int length; /**< string compare length  */
  int (*function)(const char *, struct json_object *,
                  double *); /**< c function pointer related for REST API  */
};

/**
 * @brief arguments object
 */
struct es_arg {
  char *name; /**< argument name */
  enum data_type type;
  SEXP (*converter)(); /**< function that converts c argument to R argument */
  int (*checker)();    /**< function that checks this argument */
};

/**
 * @brief http request callback function
 * @param cls
 * @param req http request
 * @param res http response
 */
void req_cb(__SG_UNUSED void *cls, struct sg_httpreq *req,
            struct sg_httpres *res);

/**
 * @brief wrapping for reach_score in R
 * @param function function name define in reach_score of estimateCVRisk
 * @param json_jo  json-c object of http request payload
 * @param score    calculated result
 */
int reach_score(const char *function, struct json_object *json_jo,
                double *score);

/**
 * @brief signal event handler
 * @param signal signal
 * @param events event
 * @param base   event base
 */
void sighandler(int signal, short events, void *base);

/**
 * @brief load R script from file
 * @param operation operation of R, "library" for loading R library, "source"
 * for loading R script
 * @param name operation name
 */
void source(const char *operation, const char *name);

/**
 * @brief API Supported functions
 */
static const struct es_function es_funs[] = {
    {.name = "reach_score_next_cv",
     .length = sizeof("reach_score_next_cv") - 1,
     .function = reach_score},
    {.name = "reach_score_cv_death",
     .length = sizeof("reach_score_cv_death") - 1,
     .function = reach_score}};

/**
 * @brief reach score arguments
 */
static const struct es_arg rsas[] = {{.name = "sex",
                                      .type = STRING,
                                      .converter = Rf_mkString,
                                      .checker = gender_checker},
                                     {.name = "age",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "smoker",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "diabetic",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "bmi",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "vasc",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "cv_event",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "chf",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "af",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "statin",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "asa",
                                      .type = INT,
                                      .converter = Rf_ScalarInteger,
                                      .checker = all_pass_checker},
                                     {.name = "region_EE_or_ME",
                                      .type = BOOL,
                                      .converter = Rf_ScalarLogical,
                                      .checker = all_pass_checker},
                                     {.name = "region_jap_aust",
                                      .type = BOOL,
                                      .converter = Rf_ScalarLogical,
                                      .checker = all_pass_checker}};

#endif
