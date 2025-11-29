FROM ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive

USER root

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y --no-install-recommends install \
        build-essential \
        software-properties-common \
        language-pack-en-base \
        git \
        git-core \
        git-lfs \
        ffmpeg \
        python3 \
        python3-dbg \
        python3-dev \
        python3-pip \
        python3-pexpect \
        python3-git \
        python3-jinja2 \
        python3-subunit \
        vim \
        wget \
        gcc \
        g++ \
        gdbserver \
        gcc-avr \
        binutils-avr \
        avr-libc \
        gdb-avr \
        libusb-dev \
        avrdude \
        can-utils \
        gdb-multiarch \
        graphviz \
        gcc-multilib \
        g++-multilib \
        stlink-tools \
        gdb-multiarch \
        can-utils \
        openocd \
        valgrind \
        libncurses5 \
        libncurses5-dev \
        cmake \
        gdb \
        clang-format \
        unzip \
        ruby \
        texlive-xetex \
        inkscape \
        libcairo2-dev \
        flex \
        bison \
        gperf \
        doxygen \
        ninja-build \
        ccache \
        libffi-dev \
        libssl-dev \
        dfu-util \
        default-jre \
        default-jdk \
        diffstat \
        chrpath \
        cpio \
        gawk \
        file \
        zstd \
        liblz4-tool \
        python3.10-venv \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get -y clean

RUN git config --global --add safe.directory /workspace

RUN ln -s /usr/bin/python3 /usr/bin/python

RUN gem install ceedling
RUN python3 -m pip install pyserial

RUN pip install gcovr
RUN pip install Pillow==9.5.0
RUN pip install json_repair==0.30.0
RUN pip install Requests==2.32.3
RUN pip install black==21.12b0
RUN pip install click==8.0.4
RUN pip install flake8==4.0.1
RUN pip install isort==6.0.1
RUN pip install pandas==2.3.0
RUN pip install xgboost==3.0.2
RUN pip install pulp==3.2.1
RUN pip install -U scikit-learn
RUN pip install ffmpeg_python==0.2.0
RUN pip install openai==1.86.0
RUN pip install opencv_python==4.10.0.84
RUN pip install matplotlib==3.10.5
RUN pip install graphviz
RUN pip install reportlab
RUN pip install google-auth==2.40.3
RUN pip install google-auth-oauthlib==1.2.2
RUN pip install google-auth-httplib2==0.2.0
RUN pip install google-api-python-client==2.172.0
RUN pip install scikit-learn==1.7.2
RUN pip install pytest
RUN pip install gTTS==2.5.4
RUN pip install googletrans==4.0.2
RUN pip install flatlib==0.2.3
RUN pip install pyswisseph==2.10.3.2
RUN pip install kerykeion==4.26.2
RUN pip install geopy==2.4.1
RUN pip install pytz==2025.2
RUN pip install timezonefinder==6.5.9
RUN pip install pylatex==1.4.2
RUN pip install numpy==2.0.2
RUN pip install skyfield==1.53
RUN pip install pikepdf==9.8.1
RUN pip install pyparsing==2.4.2

# Install non-eabi-gcc
RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar -xf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
ENV PATH="/gcc-arm-none-eabi-10.3-2021.10/bin:${PATH}"

# Install Renode
ARG RENODE_VERSION=1.16.0
RUN wget https://github.com/renode/renode/releases/download/v${RENODE_VERSION}/renode_${RENODE_VERSION}_amd64.deb && \
    apt-get update && \
    apt-get install -y --no-install-recommends ./renode_${RENODE_VERSION}_amd64.deb python3-dev && \
    rm ./renode_${RENODE_VERSION}_amd64.deb && \
    rm -rf /var/lib/apt/lists/*
RUN pip3 install -r /opt/renode/tests/requirements.txt --no-cache-dir

# ESP32 related
RUN cd / && \
    git clone -b v5.3.2 --recursive https://github.com/espressif/esp-idf.git esp-idf && \
    cd esp-idf && ./install.sh esp32,esp32s2,esp32s3 && . ./export.sh && \
    git clone https://github.com/espressif/esp32-camera.git && idf.py add-dependency "espressif/esp32-camera"

# Yocto/Bitbake related
RUN cd / && git clone git://git.openembedded.org/bitbake
ENV PATH="${PATH}:/bitbake/bin"
ENV PYTHONPATH="${PYTHONPATH}:/bitbake/lib"
RUN pip install -r bitbake/toaster-requirements.txt

# esp8266 related
RUN cd / && mkdir esp && cd esp && \
    wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz && \
    tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
ENV PATH="/esp/xtensa-lx106-elf/bin:${PATH}"

CMD ["/bin/bash"]

WORKDIR /workspace/
