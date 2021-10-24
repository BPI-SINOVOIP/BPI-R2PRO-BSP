rtc_demo :
	$(CC) -fPIC -shared -o librtc_demo.so rtc_demo.c

.PHONY : clean
clean :
	rm -f *.o *~ librtc_demo.so

.PHONY : install
install :
	cp -f S100time_syn.sh $(TARGET_DIR)/etc/init.d/
	cp -f librtc_demo.so $(TARGET_DIR)/usr/lib/
	cp -f librtc_demo.so $(STAGING_DIR)/usr/lib/
	cp -f rtc_demo.h $(STAGING_DIR)/usr/include/

.PHONY: uninstall
uninstall :
	rm -f $(TARGET_DIR)/etc/init.d/S100time_syn.sh
	rm -f $(TARGET_DIR)/usr/lib/librtc_demo.so
	rm -f $(STAGING_DIR)/usr/include/rtc_demo.h
	rm -f $(STAGING_DIR)/usr/lib/librtc_demo.so
