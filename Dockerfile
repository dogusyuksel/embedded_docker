FROM ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive

USER root

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y --no-install-recommends install \
        build-essential \
        clang-format \
        valgrind \
        gdb \
        ruby \
        git \
        git-core \
        git-lfs \
        python3-dbg \
        python3-dev \
        python3-pip \
        python3-pexpect \
        python3-git \
        python3-jinja2 \
        python3-subunit \
        vim \
        language-pack-en-base \
        tree \
        can-utils \
        socat \
        cmake \
        gcc-multilib \
        g++-multilib \
        software-properties-common \
        wget \
        openocd \
        stlink-tools \
        gdb-multiarch \
        usbutils \
        libusb-1.0-0-dev \
        default-jdk \
        gawk \
        diffstat \
        unzip \
        texinfo \
        chrpath \
        cpio \
        xz-utils \
        debianutils \
        iputils-ping \
        libegl1-mesa \
        libsdl1.2-dev \
        xterm \
        file \
        mesa-common-dev \
        zstd \
        liblz4-tool \
        bluetooth build-essential \
        libglib2.0-dev \
        libdbus-1-dev \
        libncurses-dev \
        flex \
        bison \
        gperf \
        splint && \
    apt-get -y clean

RUN python3 -m pip install pyserial

RUN gem install ceedling
RUN pip install gcovr

RUN git config --global --add safe.directory /workspace

RUN ln -s /usr/bin/python3 /usr/bin/python
RUN pip install matplotlib
RUN pip install pytest
RUN python3 -m pip install flake8

RUN cd / && \
    git clone git://git.openembedded.org/bitbake
ENV PATH="${PATH}:/bitbake/bin"
ENV PYTHONPATH="${PYTHONPATH}:/bitbake/lib"
RUN pip install -r bitbake/toaster-requirements.txt

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar -xf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
ENV PATH="/gcc-arm-none-eabi-10.3-2021.10/bin:${PATH}"

RUN pip install pyparsing==2.4.2
RUN cd / && \
    mkdir esp && \
    cd esp && \
    wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz && \
    tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
ENV PATH="/esp/xtensa-lx106-elf/bin:${PATH}"
RUN cd / && \
    git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git ESP8266_RTOS_SDK && \
    python3 -m pip install --user -r ESP8266_RTOS_SDK/requirements.txt

CMD ["/bin/bash"]

WORKDIR /workspace/

COPY ./transport.py.patch /workspace/transport.py.patch
COPY ./canard_stm32.patch /workspace/canard_stm32.patch

RUN cd /workspace && \
    mkdir thirdparty && \
    git clone https://github.com/STMicroelectronics/stm32l4xx_hal_driver.git thirdparty/STM32L4XX_HAL && \
		git clone https://github.com/modm-io/cmsis-header-stm32.git thirdparty/cmsis-header-stm32 && \
		git clone https://github.com/ARM-software/CMSIS_5.git thirdparty/CMSIS_5 && \
		git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git thirdparty/FreeRTOS-Kernel && \
		git clone https://github.com/STMicroelectronics/stm32f1xx_hal_driver.git thirdparty/STM32F103X_HAL && \
		git clone https://github.com/dogusyuksel/embedded_linting.git thirdparty/linting && \
		git clone https://github.com/mpaland/printf.git thirdparty/custom_printf && \
		git clone https://github.com/STMicroelectronics/OpenOCD.git thirdparty/openocd && \
		cat ./thirdparty/openocd/tcl/board/stm32f103c8_blue_pill.cfg | sed -e "s/set FLASH_SIZE 0x20000/set FLASH_SIZE 0x10000/" > ./thirdparty/openocd/tcl/board/stm32f103c8_custom.cfg && \
		git clone https://github.com/OpenCyphal/libcanard.git thirdparty/libcanard && \
		cd /workspace/thirdparty/libcanard && \
		git checkout 43de1c4966b8d1e5d57978949d63e697f045b358 && \
		git submodule update --init --recursive && \
  		cd /workspace && patch -d . -p1 < ./transport.py.patch && patch -d . -p1 < ./canard_stm32.patch && \
		cd /workspace && \
		git clone https://github.com/DaveGamble/cJSON.git cjson && \
		cd cjson && \
		git checkout 87d8f0961a01bf09bef98ff89bae9fdec42181ee && \
		mkdir build && cd build && cmake .. && make && \
		cd /workspace && git clone -b kirkstone git://git.yoctoproject.org/poky.git && \
		cd /workspace && git clone https://github.com/dogusyuksel/ti_cc2640r2f_sdk.git ti_cc2640r2f_sdk

