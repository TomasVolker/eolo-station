
#ifndef EOLO_MODEM_H
#define EOLO_MODEM_H

#define MODEM_IP_BUFFER 30

#define MODEM_APN_MAX_SIZE 40
#define MODEM_USERNAME_MAX_SIZE 40
#define MODEM_PASSWORD_MAX_SIZE 40

#include "board.h"

struct ModemStatus{
    Bool initialized;
    Bool simReady;
    Bool joined;
    char ip[MODEM_IP_BUFFER];
};

extern struct ModemStatus modemStatus;

void resetDataModem();
Bool checkSim();
Bool joinNetwork(const char * apn, const char * username, const char * password);
Bool startTcpConnection(const char * url, uint32_t port);
Bool tcpSendData(const char * data, uint32_t size);
const char* tcpReceiveData();
void tcpClose();

void modemManagerTask(void * a);


#endif
