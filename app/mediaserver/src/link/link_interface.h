#ifndef _RK_LINK_INTERFACE_H_
#define _RK_LINK_INTERFACE_H_

namespace rockchip {
namespace mediaserver {

class LinkInterFace {
public:
  LinkInterFace() {}
  virtual ~LinkInterFace() {}
  virtual void FillMediaParam(MediaParamType param, unsigned int value) = 0;
  virtual int FillLicenseKey(pLicenseKey plicense) = 0;
  virtual int InitDevice() = 0;
  virtual int DeInitDevice() = 0;
  virtual int StartLink() = 0;
  virtual int StopLink() = 0;

private:
};

class LinkVirApi {
public:
  LinkVirApi() {}
  virtual ~LinkVirApi() {}
  virtual void FillMediaParam(MediaParamType param, unsigned int value){};
  virtual int FillLicenseKey(pLicenseKey plicense) { return 0; };
  virtual int InitDevice() { return 0; };
  virtual int DeInitDevice() { return 0; };
  virtual int StartLink() { return 0; };
  virtual int StopLink() { return 0; };

  static void PushVideoHandler(unsigned char *buffer, unsigned int buffer_size,
                               int64_t present_time, int nal_type) {}
  static void PushAudioHandler(unsigned char *buffer, unsigned int buffer_size,
                               int64_t present_time) {}
  static void PushCaptureHandler(unsigned char *buffer,
                                 unsigned int buffer_size, int type,
                                 const char *id) {}

private:
};

} // namespace mediaserver
} // namespace rockchip

#endif
