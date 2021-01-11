#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <tinyxml2.h>

#ifndef STASSID
#define STASSID "JUMP"
#define STAPSK  "025260652"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "ESP8266-UNO";

//===============================================

using namespace tinyxml2;

XMLDocument xmlDocument;
XMLDocument doc;

//char * testDocument = "<root><element>7</element></root>";
//String testDocument = "<root><element>7</element></root>";

//read
//String testDocument = "<confirmed-RequestPDU><!-- this is comment 1 --><element>7</element><invokeID>2676121</invokeID><!-- this is comment 2 --><ConfirmedServiceRequest><read specificationWithResult=\"false\"><variableAccessSpecification><listOfVariable><SEQUENCE><variableSpecification><name><domain-specific><domainId>IEDNameLDINst</domainId><itemId>LLN0$ST$Beh$stVal</itemId></domain-specific></name></variableSpecification></SEQUENCE></listOfVariable></variableAccessSpecification></read></ConfirmedServiceRequest></confirmed-RequestPDU>";

//write
//String testDocument = "<confirmed-RequestPDU><!-- this is comment 1 --><element>7</element><invokeID>2676121</invokeID><!-- this is comment 2 --><ConfirmedServiceRequest><write><variableAccessSpecification><listOfVariable><SEQUENCE><variableSpecification><name><domain-specific><domainId>IEDNameLDINst</domainId><itemId>LLN0$ST$Beh$stVal</itemId></domain-specific></name></variableSpecification></SEQUENCE></listOfVariable></variableAccessSpecification><listOfData><Data><integer>1</integer></Data></listOfData></write></ConfirmedServiceRequest></confirmed-RequestPDU>";
String testDocument = "<iq type=\"get\" id=\"444555\" to=\"esp@xmpp.egat.co.th/8n1x08ixsd\" from=\"vscode@xmpp.egat.co.th/61850Client\"><query xmlns=\"jabber:iq:iq\"><confirmed-RequestPDU><invokeID>1234588</invokeID><ConfirmedServiceRequest><read specificationWithResult=\"false\"><variableAccessSpecification><listOfVariable><SEQUENCE><variableSpecification><name><domain-specific><domainId>IEDNameLDINst</domainId><itemId>LLN0$ST$Beh$stVal</itemId></domain-specific></name></variableSpecification></SEQUENCE></listOfVariable></variableAccessSpecification></read></ConfirmedServiceRequest></confirmed-RequestPDU></query></iq>";



int val;

//===============================================

int led_pin = 13;
#define N_DIMMERS 3
int dimmer_pin[] = {14, 5, 15};

String DomainId;
String ItemId;
String InvokeID;
String DataInteger;


Ticker eventTick1;
Ticker eventTick2;

void setup() {
    Serial.begin(115200);

    /* switch on led */
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

    Serial.println("Booting");
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        Serial.println("Retrying connection...");
    }
    /* switch off led */
    digitalWrite(led_pin, HIGH);

    /* configure dimmers, and OTA server events */
    analogWriteRange(1000);
    analogWrite(led_pin, 990);

    for (int i = 0; i < N_DIMMERS; i++) {
        pinMode(dimmer_pin[i], OUTPUT);
        analogWrite(dimmer_pin[i], 50);
    }

    ArduinoOTA.setHostname(host);
    ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
        for (int i = 0; i < N_DIMMERS; i++) {
            analogWrite(dimmer_pin[i], 0);
        }
        analogWrite(led_pin, 0);
    });

    ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
        for (int i = 0; i < 30; i++) {
            analogWrite(led_pin, (i * 100) % 1001);
            delay(50);
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        (void)error;
        ESP.restart();
    });

    /* setup the OTA server */
    ArduinoOTA.begin();
    Serial.println("Ready");

    eventTick1.attach(20, eventProcess1);//2 sec
    eventTick2.attach(5, eventProcess2);//5 sec

    if (xmlDocument.Parse(testDocument.c_str()) != XML_SUCCESS) {
        Serial.println("Error parsing");
        return;
    };
}

void loop() {
    ArduinoOTA.handle();
}

void eventProcess1() {
    boolean attributeBool;
    const char* attributeString;
    uint32_t  attributeUint32;
    unsigned long n1Time = millis();

    XMLElement* root = xmlDocument.RootElement();
    if (NULL != root) {
        const char* rootName = root->Name();

        if (strcmp(rootName, "iq") == 0) {//iq query confirmed-RequestPDU

            if (root->QueryStringAttribute("type", &attributeString) != XML_SUCCESS) {
                Serial.println("Could not obtain the attribute");
            } else {
                Serial.print("type: ");
                Serial.println(attributeString);
            }

            if (root->QueryUnsignedAttribute("id", &attributeUint32) != XML_SUCCESS) {
                Serial.println("Could not obtain the attribute");
            } else {
                Serial.print("id: ");
                Serial.println(attributeUint32);
            }

            

            if (root->QueryStringAttribute("to", &attributeString) != XML_SUCCESS) {
                Serial.println("Could not obtain the attribute");
            } else {
                Serial.print("to: ");
                Serial.println(attributeString);
            }

            if (root->QueryStringAttribute("from", &attributeString) != XML_SUCCESS) {
                Serial.println("Could not obtain the attribute");
            } else {
                Serial.print("from: ");
                Serial.println(attributeString);
            }

            XMLElement* readWriteElement = root->FirstChildElement("query")->FirstChildElement("confirmed-RequestPDU")->FirstChildElement("ConfirmedServiceRequest")->FirstChildElement();
            if (NULL != readWriteElement) {
                const char* strTagName = readWriteElement->Name();

                if (strcmp(strTagName, "read") == 0) {
                    XMLElement* domain_specific = readWriteElement->FirstChildElement("variableAccessSpecification")->FirstChildElement("listOfVariable")->FirstChildElement("SEQUENCE")->FirstChildElement("variableSpecification")->FirstChildElement("name")->FirstChildElement("domain-specific");
                    if (NULL != domain_specific) {
                        XMLElement* domainIdElement = domain_specific->FirstChildElement("domainId");
                        if (NULL != domainIdElement) {
                            const char* domainId = domainIdElement->GetText();
                            DomainId = String(domainId);
                            Serial.println(DomainId);
                        }

                        XMLElement* itemIdElement = domain_specific->FirstChildElement("itemId");
                        if (NULL != itemIdElement) {
                            const char* itemId = itemIdElement->GetText();
                            ItemId = String(itemId);
                            Serial.println(ItemId);
                        }
                    }

                    if (readWriteElement->QueryBoolAttribute("specificationWithResult", &attributeBool) != XML_SUCCESS) {
                        Serial.println("Could not obtain the attribute");
                    } else {
                        Serial.print("attr: ");
                        Serial.println(attributeBool);
                    }

                } else if (strcmp(strTagName, "write") == 0) {
                    XMLElement* domain_specific = readWriteElement->FirstChildElement("variableAccessSpecification")->FirstChildElement("listOfVariable")->FirstChildElement("SEQUENCE")->FirstChildElement("variableSpecification")->FirstChildElement("name")->FirstChildElement("domain-specific");
                    if (NULL != domain_specific) {
                        XMLElement* domainIdElement = domain_specific->FirstChildElement("domainId");
                        if (NULL != domainIdElement) {
                            const char* domainId = domainIdElement->GetText();
                            DomainId = String(domainId);
                            Serial.println(DomainId);
                        }

                        XMLElement* itemIdElement = domain_specific->FirstChildElement("itemId");
                        if (NULL != itemIdElement) {
                            const char* itemId = itemIdElement->GetText();
                            ItemId = String(itemId);
                            Serial.println(ItemId);
                        }
                    }

                    XMLElement* integerElement = readWriteElement->FirstChildElement("listOfData")->FirstChildElement("Data")->FirstChildElement("integer");
                    if (NULL != integerElement) {
                        const char* dataInteger = integerElement->GetText();
                        DataInteger = String(dataInteger);
                        Serial.println(DataInteger);
                    }

                } else {
                    Serial.println("unknown function");
                }


                XMLElement* invokeIdElement = root->FirstChildElement("query")->FirstChildElement("confirmed-RequestPDU")->FirstChildElement("invokeID");
                if (NULL != invokeIdElement) {
                    const char* invokeID = invokeIdElement->GetText();
                    InvokeID = String(invokeID);
                    Serial.println(InvokeID);
                } else {
                    Serial.println(F("query not found"));
                }
            }


        } else { //not iq

        }
    }


    Serial.println();

    XMLPrinter printer;
    xmlDocument.Print(&printer);
    Serial.println(printer.CStr());


    unsigned long n2Time = millis();
    Serial.println(n2Time - n1Time);
    Serial.println();

}
void eventProcess2() {
    unsigned long n3Time = millis();

    //XMLDocument doc;
    XMLElement* pRoot = doc.RootElement();
    if (NULL == pRoot) {

        pRoot = doc.NewElement("confirmed-ResponsePDU");
        //XMLComment* p1Element = doc.NewComment("invokeId must the same");
        XMLElement* n1Element = doc.NewElement("invokeID");
        n1Element->SetText("2676121");
        pRoot->InsertFirstChild(n1Element);
        //pRoot->InsertFirstChild(p1Element);
        //pRoot->InsertAfterChild(p1Element, n1Element);

        n1Element = doc.NewElement("ConfirmedServiceResponse");
        XMLElement* n2Element = doc.NewElement("read");
        XMLElement* n3Element = doc.NewElement("listOfAccessResult");
        XMLElement* n4Element = doc.NewElement("AccessResult");
        XMLElement* n5Element = doc.NewElement("success");
        XMLElement* n6Element = doc.NewElement("integer");

        n6Element->SetText("1");//SetText("1"),SetText(1.0f)

        pRoot->InsertEndChild(n1Element);
        n1Element->InsertEndChild(n2Element);
        n2Element->InsertEndChild(n3Element);
        n3Element->InsertEndChild(n4Element);
        n4Element->InsertEndChild(n5Element);
        n5Element->InsertEndChild(n6Element);


        doc.InsertFirstChild(pRoot);
    }

    Serial.println();


    XMLPrinter printer2;
    doc.Print(&printer2);
    Serial.println(printer2.CStr());

    unsigned long n4Time = millis();
    Serial.println(n4Time - n3Time);
    Serial.println();
}
