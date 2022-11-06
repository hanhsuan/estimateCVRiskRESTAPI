FROM r-base AS builder

RUN apt-get update && apt-get install -y libxml2-dev libcurl4-openssl-dev libfontconfig1-dev libssl-dev libgit2-dev libharfbuzz-dev libfribidi-dev libfreetype6-dev libpng-dev libtiff5-dev libjpeg-dev gcc make libgnutls28-dev cmake git curl libevent-dev libjson-c-dev

RUN echo 'install.packages("usethis")' >> install.R && \
    echo 'install.packages("pkgdown")' >> install.R && \
    echo 'install.packages("rcmdcheck")' >> install.R && \
    echo 'install.packages("roxygen2")' >> install.R && \
    echo 'install.packages("rversions")' >> install.R && \
    echo 'install.packages("urlchecker")' >> install.R && \
    echo 'install.packages("devtools")' >> install.R && \
    echo 'devtools::install_github("DGruen89/estimateCVRisk")' >> install.R && \
    Rscript install.R

RUN git clone https://github.com/risoflora/libsagui.git && cd libsagui && \
    cmake -DBUILD_TESTING=ON && make all && \
    cp -L /libsagui/src/$(objdump -p /libsagui/src/libsagui.so | grep SONAME | sed 's/ //g' | sed 's/SONAME//g') / && \
    cp /libsagui.so* /usr/lib/ && cp /libsagui.so* /usr/lib/libsagui.so && \
    mkdir -p /usr/include/sagui/ && cp /libsagui/include/sagui.h /usr/include/sagui/sagui.h

COPY ./ /

RUN make clean main

FROM r-base

RUN apt-get update && apt-get -y  install libevent-dev libjson-c-dev

COPY --from=builder /main /main

COPY --from=builder /libsagui.so* /lib

COPY --from=builder /usr/local/lib/R/site-library /usr/local/lib/R/site-library

ENV R_HOME=/usr/lib/R R_LIBS=/usr/local/lib/R/site-library/

RUN echo '#!/bin/sh' > /start.sh && \
    echo '_term() { ' >> /start.sh && \
    echo '    kill -TERM "${child}" 2>/dev/null' >> /start.sh && \
    echo '}' >> /start.sh && \
    echo 'trap _term TERM' >> /start.sh && \
    echo 'prlimit --stack=unlimited --pid $$; ulimit -s unlimited' >> /start.sh && \
    echo '/main $1 &' >> /start.sh && \
    echo 'child=$!' >> /start.sh && \
    echo 'wait "${child}"' >> /start.sh && \
    chmod -R 777 /start.sh

ENTRYPOINT ["/start.sh"]
CMD ["/start.sh"]
