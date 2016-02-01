
struct dsgccClientNotification {
  int eventType;
  int eventValue;
  int clientPort;
};

program DSGCCSTATUSCALLBACK {
  version DSGCCSTATUSCALLBACKVERS {
    int DSGCCCLIENTNOTIFICATION(struct dsgccClientNotification) = 1;
  } = 1;
} = 0x05082002;
