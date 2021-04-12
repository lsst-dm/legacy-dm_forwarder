ARG FWD_BASE_VERSION
FROM ts-dockerhub.lsst.org/lsstts/fwd_base:${FWD_BASE_VERSION}

RUN groupadd -g 61003 iip
RUN useradd -u 61003 -g 61003 iip

USER iip

ENTRYPOINT [ "tail", "-f", "/dev/null" ]
#CMD ["/app/bin/dm_forwarder"]

