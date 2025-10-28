FROM docker.io/zephyrprojectrtos/ci:v0.28.2

# Defined from https://github.com/zephyrproject-rtos/docker-image/blob/main/Dockerfile.devel
ARG USERNAME=user

# Install additional tools for embedded development
USER root
RUN apt-get update && apt-get install -y \
    clangd \
    python3-dev \
    python3-pip \
    openocd \
    gdb-multiarch \
    stlink-tools \
    minicom \
    screen \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for development
RUN pip3 install --no-cache-dir \
    pyserial \
    pyyaml \
    west \
    intelhex \
    jsonschema

# Switch back to user context and set working directory
USER $USERNAME
WORKDIR /workdir

# Add bash history setup using dev directory
RUN mkdir -p /home/$USERNAME/dev && \
    touch /home/$USERNAME/dev/.bash_history && \
    chmod 600 /home/$USERNAME/dev/.bash_history && \
    echo 'export HISTFILE=~/dev/.bash_history' >> /home/$USERNAME/.bashrc && \
    echo 'export HISTSIZE=1000' >> /home/$USERNAME/.bashrc && \
    echo 'export HISTFILESIZE=2000' >> /home/$USERNAME/.bashrc && \
    echo 'shopt -s histappend' >> /home/$USERNAME/.bashrc && \
    echo 'PROMPT_COMMAND="history -a; $PROMPT_COMMAND"' >> /home/$USERNAME/.bashrc

# Add development environment setup
RUN echo 'export ZEPHYR_BASE=/workdir/zephyr' >> /home/$USERNAME/.bashrc && \
    echo 'export ZEPHYR_TOOLCHAIN_VARIANT=zephyr' >> /home/$USERNAME/.bashrc && \
    echo 'export ZEPHYR_SDK_INSTALL_DIR=/opt/toolchains/zephyr-sdk-0.17.2' >> /home/$USERNAME/.bashrc && \
    echo 'export PATH=$PATH:/opt/toolchains/zephyr-sdk-0.17.2/arm-zephyr-eabi/bin' >> /home/$USERNAME/.bashrc

# Create a welcome message for the development environment
RUN echo 'echo "=== Team 8 Circuiteers - Embedded Development ==="' >> /home/$USERNAME/.bashrc && \
    echo 'echo "Default Board: ${BOARD:-nucleo_wl55jc}"' >> /home/$USERNAME/.bashrc && \
    echo 'echo "Zephyr SDK: $ZEPHYR_SDK_INSTALL_DIR"' >> /home/$USERNAME/.bashrc && \
    echo 'echo "Use: west build -p always -b BOARD apps/APP_NAME"' >> /home/$USERNAME/.bashrc && \
    echo 'echo "Initialize: west init && west update && west zephyr-export"' >> /home/$USERNAME/.bashrc && \
    echo 'echo "==============================================="' >> /home/$USERNAME/.bashrc
