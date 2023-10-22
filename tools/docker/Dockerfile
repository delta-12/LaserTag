FROM espressif/idf:latest
WORKDIR /opt
RUN git clone https://github.com/danmar/cppcheck.git
RUN mkdir /opt/cppcheck/build
WORKDIR /opt/cppcheck/build
RUN /opt/esp/tools/cmake/3.24.0/bin/cmake ..
RUN /opt/esp/tools/cmake/3.24.0/bin/cmake --build .
WORKDIR /project
