# Use a base image that supports ARM64 architecture (e.g., for 64-bit Raspberry Pi)
FROM arm64v8/ubuntu:latest

# Install curl to test network connectivity (optional, for troubleshooting)
RUN apt-get update

# Install ip command, required by set_eui.sh
RUN apt install -y iproute2

# Copy the script to the container
COPY . /root/.

# Make the scripts executable
RUN chmod +x /root/lora_pkt_fwd
RUN chmod +x /root/reset_lgw.sh
RUN chmod +x /root/set_eui.sh
RUN chmod +x /root/start.sh
RUN chmod +x /root/update_gwid.sh

# Set the working directory
WORKDIR /root

# Define the entry point
CMD ["bash", "./start.sh"]