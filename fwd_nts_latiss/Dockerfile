ARG FWD_BASE_VERSION
FROM ts-dockerhub.lsst.org/lsstts/fwd_base:${FWD_BASE_VERSION}

RUN groupadd -g 985 iip
RUN useradd -u 989 -g 985 iip

USER iip

ENTRYPOINT [ "tail", "-f", "/dev/null" ]
#CMD ["/app/bin/dm_forwarder"]

