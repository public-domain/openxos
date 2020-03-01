.m86.obj:
	xmac $(@DPN)

.c.obj:
	WCC386 /D1 /I=XOSINC:\WCX\ /ZQ /3S /J /W3 /WE /FP3 /S \
		/NM=$(@DPN) $(@DPN)

#	wcc386 /D1 /MS /I=xosinc:\wcx\ /NM=$(@DPN) $(@DPN)

all:		xunzip.run xzip.run
	@echo All done

xunzip.run:	xunzip.obj unstorefile.obj inflatefile.obj inflate.obj \
		infblock.obj inftrees.obj infcodes.obj inffast.obj \
		infutil.obj crc32.obj adler32.obj
	xlink xoslib:\xos\defsegs xunzip unstorefile inflatefile \
		inflate infblock inftrees infcodes inffast infutil \
		crc32 adler32 \
		XOSLIB:\xos\libx01 XOSLIB:\xos\libc01 \
		/OUTPUT=xunzip /MAP=xunzip /SYM=xunzip
	copy /OVER xunzip.run newcmd:xunzip.run

xunzip.obj:	xunzip.c
unstorefile.obj: unstorefile.c
inflatefile.obj: inflatefile.c
inflate.obj:	inflate.c
infblock.obj:	infblock.c
inftrees.obj:	inftrees.c
infcodes.obj:	infcodes.c
inffast.obj:	inffast.c
infutil.obj:	infutil.c
crc32.obj:	crc32.m86

xzip.run:	xzip.obj deflatefile.obj storefile.obj deflate.obj \
		trees.obj zerror.obj crc32.obj adler32.obj
	xlink xoslib:\xos\defsegs xzip storefile deflatefile crc32 \
		deflate trees zerror adler32 \
		XOSLIB:\xos\libx01 XOSLIB:\xos\libc01 \
		/OUTPUT=xzip /MAP=xzip /SYM=xzip
	copy /OVER xzip.run newcmd:xzip.run

xzip.obj:	xzip.c
deflatefile.obj: deflatefile.c
storefile.obj:	storefile.c
deflate.obj:	deflate.c
trees.obj:	trees.c
zerror.obj:	zerror.c
adler32.obj:	adler32.c
