# Multi Variant Analysis using SIMCA-QP

The libchemgps library is a generic library for making predictions using
SIMCA-QP. Use this library to write standalone and client/server
applications for SIMCA-QP. The chemgps-sqp2 package contains both a fully
functional standalone application as well as an client/server solution.

You need a separate license from Umetrics (http://www.umetrics.com) for 
using this library.

### BUILD:

Use CC to link against 32-bit library on 64-bit Linux:

```bash
CC="gcc32" CFLAGS="-Wall -O2 -m32" ./configure
make
make install
```

### EXAMPLE:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chemgps.h>

/*
 * Using callback functions cgps_syslog() and cgps_load_data()
 * defined elsewhere. Error handling at minimum for readability.
 */

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main(void) {
    struct cgps_options opts;
    struct cgps_project proj;
    struct cgps_predict pred;
    struct cgps_result res;
    const char *path = "/tmp/proj.usp";
    int i, model;

    memset(&opts, 0, sizeof (struct cgps_options));

    opts.logger = cgps_syslog;
    opts.indata = cgps_load_data;

    if (cgps_project_load(&proj, path, &opts) == 0) {
        for (i = 1; i <= proj.models; ++i) {
            cgps_predict_init(&proj, &pred);
            if ((model = cgps_predict(&proj, i, &pred)) != -1) {
                if (cgps_result_init(&proj, &res) == 0) {
                    if (cgps_result(&proj, model, &pred, &res, stdout) != 0) {
                        die("get result failed");
                    }
                    cgps_result_cleanup(&proj, &res);
                } else {
                    die("initilize result failed");
                }
            } else {
                die("predict failed");
            }
        }
    } else {
        die("failed load project");
    }

    return 0;
}
```

### ABOUT:
Used in backend code by https://chemgps.bmc.uu.se for PCA. You need to obtain 
a server side license for SIMCA-QP to use these binaries.
