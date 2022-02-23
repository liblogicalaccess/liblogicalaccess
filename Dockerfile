FROM debian:bullseye
RUN apt-get update
RUN apt-get install --no-install-recommends -y git gcc g++ make cmake pkgconf python3 python3-pip libpcsclite-dev libusb-dev
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1
RUN pip install conan

# Volume to repository root of LLA project
VOLUME /lla

# Volume to ~/.conan/data to get built package on host computer.
RUN ln -s /root/.conan/data /conan_data
VOLUME /conan_data

WORKDIR /lla
CMD (conan create .)
