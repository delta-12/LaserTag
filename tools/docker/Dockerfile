FROM espressif/idf:latest
WORKDIR /opt
RUN git clone https://github.com/danmar/cppcheck.git
RUN mkdir /opt/cppcheck/build
WORKDIR /opt/cppcheck/build
RUN /opt/esp/tools/cmake/3.24.0/bin/cmake ..
RUN /opt/esp/tools/cmake/3.24.0/bin/cmake --build .
WORKDIR /opt
RUN git clone https://github.com/doxygen/doxygen.git
RUN mkdir /opt/doxygen/build
WORKDIR /opt/doxygen/build
RUN /opt/esp/tools/cmake/3.24.0/bin/cmake -G "Unix Makefiles" ..
RUN make
RUN make install
WORKDIR /project
