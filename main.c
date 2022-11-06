#include "es.h"
#include <Rembedded.h>
#include <Rinternals.h>
#include <signal.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  struct sg_httpsrv *srv;
  uint16_t port;
  struct event ev_sigterm;
  struct event ev_sigint;
  struct event_base *ev_base;

  if (argc != 2) {
    printf("%.6s <PORT>\n", argv[0]);
    return EXIT_FAILURE;
  }

  port = strtol(argv[1], NULL, 10);
  srv = sg_httpsrv_new(req_cb, NULL);
  if (!sg_httpsrv_listen(srv, port, true)) {
    sg_httpsrv_free(srv);
    return EXIT_FAILURE;
  }

  // Intialize the R environment.
  char *r_argv[] = {"R", "--vanilla", "--no-echo", "--silent"};
  Rf_initEmbeddedR(sizeof(r_argv) / sizeof(r_argv[0]), r_argv);

  source("options", "warn=1");
  // Invoke a library in R
  source("library", "estimateCVRisk");

  // set event loop
  ev_base = event_init();

  evsignal_set(&ev_sigterm, SIGTERM, sighandler, ev_base);
  evsignal_add(&ev_sigterm, NULL);
  evsignal_set(&ev_sigint, SIGINT, sighandler, ev_base);
  evsignal_add(&ev_sigint, NULL);
  event_dispatch();

  // Release R environment
  Rf_endEmbeddedR(0);

  // Release sagui http server object
  sg_httpsrv_shutdown(srv);
  sg_httpsrv_free(srv);

  return EXIT_SUCCESS;
}
