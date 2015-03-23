build: lib/libhelpers.so cat/cat revwords/revwords lenwords/lenwords

lib/libhelpers.so cat/cat revwords/revwords:
	$(MAKE) -C $(dir $@)

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C cat clean

rebuild: clean build
