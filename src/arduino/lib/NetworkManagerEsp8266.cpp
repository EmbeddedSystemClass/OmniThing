#if defined(ARDUINO_ARCH_ESP8266)

#include "NetworkManagerEsp8266.h"
#include "OmniThing.h"
#include "OmniUtil.h"
#include "Logger.h"
#include "TriggerableFunction.h"
#include <string.h>
#include "frozen.h"

namespace omni
{
//private
    NetworkManagerEsp8266::NetworkManagerEsp8266():
        m_nDestPort(0),
        m_WebServer(nullptr),
        m_bClearBuffer(false)
    {
        m_JsonBuf[0] = 0;
        m_DestIp[0] = 0;

        WiFi.mode(WIFI_STA);


        // register trigger for printing debug info
        TriggerableFunction* tf = new TriggerableFunction(printDebug);
        OmniThing::getInstance().addTrigger(tf, 20000, nullptr, true);
    }

    void NetworkManagerEsp8266::waitUntilConnected()
    {
        if(WiFi.status() == WL_CONNECTED)
            return;

        LOG << F("Waiting to connect to access point\n");
        while(WiFi.status() != WL_CONNECTED)
        {
            sleepMillis(500);
            LOG << F(".");
        }
        LOG << Logger::endl;
    }

    void NetworkManagerEsp8266::serverRun()
    {
        if(m_bClearBuffer)
        {
            m_JsonBuf[0] = 0;
            m_bClearBuffer = false;
        }

        if(!m_WebServer)
            return;

        m_WebServer->handleClient();
    }

    void NetworkManagerEsp8266::handleConnection()
    {
        auto& manager = getInstance();  

        auto& server = *(manager.m_WebServer);

        if(!server.hasArg("plain"))
        {
            LOG << F("received no body\n");
            return;
        }

        strncpy(manager.m_JsonBuf, server.arg("plain").c_str(), OMNI_ESP_JSON_BUF_SIZE);

        server.send(200, "text/plain", "ok\n");
    }

    void NetworkManagerEsp8266::scanForCredentials(const char* json)
    {
        unsigned int len = strlen(json);
        
        char ssid[50];
        char passwd[50];

        if(json_scanf(json, len, "{ssid: %s, password: %s}", ssid, passwd) == 2)
        {
            LOG << F("applying wifi credentials ssid=") << ssid << F(" password=") << passwd << Logger::endl;
            getInstance().setCredentials(ssid, passwd);
        }

    }

//protected
//public
    NetworkManagerEsp8266::~NetworkManagerEsp8266()
    {

    }

    NetworkManagerEsp8266& NetworkManagerEsp8266::getInstance()
    {
        static NetworkManagerEsp8266* instance = nullptr;

        if(!instance)
        {
            instance = new NetworkManagerEsp8266();
        }
            
        return *instance;
    }

    void NetworkManagerEsp8266::printDebug()
    {
        auto& manager = getInstance();
        LOG << F("WiFi connection info:\n");
        LOG << F("localIP = ") << WiFi.localIP().toString().c_str() << Logger::endl;
        LOG << F("MAC address = ") << WiFi.macAddress().c_str() << Logger::endl;
        LOG << F("SSID = ") << WiFi.SSID().c_str() << Logger::endl;
        LOG << F("destIP = ") << manager.m_DestIp << Logger::endl;
        LOG << F("destPort = ") << manager.m_nDestPort << Logger::endl;
        LOG << F("\n\n");
    }

    void NetworkManagerEsp8266::setCredentials(const char* ssid, const char* passwd)
    {
        WiFi.begin(ssid, passwd);
    }

    void NetworkManagerEsp8266::setDestIp(const char* ip)
    {
        strcpy(m_DestIp, ip);
    }

    void NetworkManagerEsp8266::setDestPort(unsigned int port)
    {
        m_nDestPort = port;
    }

    void NetworkManagerEsp8266::setServerPort(unsigned int port)
    {
        if(m_WebServer)
        {
            LOG << F("deleting current web server\n");
            delete m_WebServer;
        }

        m_WebServer = new ESP8266WebServer(port);
    }
    
    const char* NetworkManagerEsp8266::getJsonString()
    {
        if(m_JsonBuf[0] == 0)
            return nullptr;

        m_bClearBuffer = true;
        return m_JsonBuf;
    }

    void NetworkManagerEsp8266::sendJson(const char* json)
    {
        LOG << F("Sending json: ") << json << Logger::endl;
        WiFiClient client;
        if(!client.connect(m_DestIp, m_nDestPort))
        {
            LOG << F("Connection to address=") << m_DestIp << F(" port=") << m_nDestPort << F(" failed\n");
            return;
        }

        client.print(String("POST ") + m_DestIp + " HTTP/1.1\r\n" +
                        "Host: " + m_DestIp + "\r\n" +
                        "Content-Length: " + strlen(json) + "\r\n" +
                        "Content-Type: " + "application/json\r\n" +
                        "Connection: close\r\n\r\n" +
                        json);

        unsigned long timeout = getMillis(); 
        while(!client.available())
        {
            if(getMillis() - timeout > 3000)
            {
                LOG << F("Post request timed out\n");
                client.stop();
                return;
            }
        }

        LOG << F("Reply:\n");
        while(client.available())
        {
            LOG << static_cast<char>(client.read());
        }
        LOG << Logger::endl;
    }

    NetworkReceiver* NetworkManagerEsp8266::createReceiverFromJson(const char* json)
    {
        unsigned int len = strlen(json);

        unsigned int port;

        if(json_scanf(json, len, "{port: %u}", &port) != 1)
            return nullptr;

        scanForCredentials(json);

        getInstance().setServerPort(port);
        return &getInstance();
    }

    NetworkSender* NetworkManagerEsp8266::createSenderFromJson(const char* json)
    {
        unsigned int len = strlen(json);

        char ip[50];
        unsigned int port;

        if(json_scanf(json, len, "{port: %u, ip: %s}", &port, ip) != 2)
            return nullptr;

        scanForCredentials(json);

        getInstance().setDestIp(ip);
        getInstance().setDestPort(port);

        return &getInstance();
    }

    const char* NetworkManagerEsp8266::Type = "NetworkManagerEsp8266";
    ObjectConfig<NetworkReceiver> NetworkManagerEsp8266::NetworkReceiverConf(Type, createReceiverFromJson);
    ObjectConfig<NetworkSender> NetworkManagerEsp8266::NetworkSenderConf(Type, createSenderFromJson);
}


#endif