all:
	(cd source; touch *.c; make)

clean:
	(cd source; rm -rf *.o)
	(cd tmp; rm -rf *)
	rm -rf CalServer

update:
	(cp mainDA_* /root/formation_data/config/caliData/)
