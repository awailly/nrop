# nrop
#
# VERSION               1.0
FROM      debian:testing
MAINTAINER Anthony Verez <averez@google.com>

RUN apt-get update -qq
RUN apt-get install -y git make python gcc g++ python-pkgconfig libz-dev \
	libglib2.0-dev dh-autoreconf libc6-dev-i386 wget unzip llvm libncurses5-dev \
	&& apt-get clean
ADD . /nrop
RUN cd /nrop/qemu && git apply ../qemu.noprologet.patch \
	&& PYTHON=$(which python2) ./configure --target-list=x86_64-linux-user --disable-sparse --disable-strip --disable-werror --disable-sdl --disable-gtk --disable-virtfs --disable-vnc --disable-cocoa --disable-xen --disable-xen-pci-passthrough --disable-brlapi --disable-vnc-tls --disable-vnc-sasl --disable-vnc-jpeg --disable-vnc-png --disable-vnc-ws --disable-curses --disable-curl --disable-fdt --disable-bluez --disable-slirp --disable-kvm --disable-rdma --disable-system --disable-guest-base --disable-pie --disable-uuid --disable-vde --disable-netmap --disable-linux-aio --disable-cap-ng --disable-attr --disable-blobs --disable-docs --disable-vhost-net --disable-spice --disable-libiscsi --disable-libnfs --disable-smartcard-nss --disable-libusb --disable-usb-redir --disable-guest-agent --disable-seccomp --disable-coroutine-pool --disable-glusterfs --disable-libssh2 --disable-vhdx --disable-quorum --disable-bsd-user \
	&& make -j32
RUN cd /nrop \
	&& sed '12s/.*/INCLUDES\=\-Ixed2\-intel64\/include\ \-I\.\ \-Iplugins\ \-Iparsers\ \-isystem\ qemu\/tcg\/i386\ \-isystem\ qemu\/x86\_64\-linux\-user\ \-isystem\ qemu\/target\-i386\ \-isystem\ qemu\ \-isystem\ qemu\/include\ \`pkg\-config\ \-\-cflags\ glib\-2\.0\`/' Makefile > Makefile2 \
	&& sed 's/\/usr\/lib\/glib\-2\.0\/include\/glibconfig\.h/\/usr\/lib\/x86\_64\-linux\-gnu\/glib\-2\.0\/include\/glibconfig\.h/' Makefile2 > Makefile \
	&& wget --output-document z3-4.3.3.005bb82a1751-x64-debian-7.7.zip "http://download-codeplex.sec.s-msft.com/Download/Release?ProjectName=z3&DownloadId=929550&FileTime=130599035112670000&Build=20941" \
	&& unzip z3-4.3.3.005bb82a1751-x64-debian-7.7.zip \
	&& cp -rf z3-4.3.3.005bb82a1751-x64-debian-7.7/include/* /usr/include/ \
	&& cp -rf z3-4.3.3.005bb82a1751-x64-debian-7.7/bin/*.so /usr/lib \
	&& bash -c "( make -j32 ); if [ $? > 0 ]; then echo $?; fi"
RUN cd /nrop/qemu && git apply ../qemu.patch \
	&& cd .. \
	&& wget http://software.intel.com/sites/landingpage/pintool/downloads/pin-2.13-65163-gcc.4.4.7-linux.tar.gz \
	&& tar xvzf pin-2.13-65163-gcc.4.4.7-linux.tar.gz \
	&& cp -r pin-2.13-65163-gcc.4.4.7-linux/extras/xed2-i* . \
	&& rm -rf pin-2.13-65163-gcc.4.4.7-linux* \
	&& sed -i 's/main/ma1n/g' qemu/x86_64-linux-user/linux-user/main.o \
	&& sed -i 's/use_icount/use_1count/g' qemu/stubs/cpu-get-icount.o \
	&& make -j32

WORKDIR /nrop

