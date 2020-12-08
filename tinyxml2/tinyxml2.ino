#include <tinyxml2.h>

using namespace tinyxml2;

XMLDocument xmlDocument;
//char * testDocument = "<root><element>7</element></root>";
//String testDocument = "<root><element>7</element></root>";
String testDocument = "<confirmed-RequestPDU><!-- this is comment 1 --><element>7</element><invokeID>2676121</invokeID><!-- this is comment 2 --><ConfirmedServiceRequest><read specificationWithResult=\"false\"><variableAccessSpecification><listOfVariable><SEQUENCE><variableSpecification><name><domain-specific><domainId>IEDNameLDINst</domainId><itemId>LLN0$ST$Beh$stVal</itemId></domain-specific></name></variableSpecification></SEQUENCE></listOfVariable></variableAccessSpecification></read></ConfirmedServiceRequest></confirmed-RequestPDU>";

int val;

void setup() {
    Serial.begin(115200);
    
    if (xmlDocument.Parse(testDocument.c_str()) != XML_SUCCESS) {
        Serial.println("Error parsing");
        return;
    };

    XMLElement* root = xmlDocument.RootElement();//confirmed-RequestPDU
    XMLElement* element = root->FirstChildElement("element");

    //int val;
    element->QueryIntText(&val);

    Serial.println(val);
}

void loop() {
    XMLElement* root = xmlDocument.RootElement();//confirmed-RequestPDU
    XMLElement* readElement = root->FirstChildElement("ConfirmedServiceRequest")->FirstChildElement("read");
    XMLElement* domain_specific = readElement->FirstChildElement("variableAccessSpecification")->FirstChildElement("listOfVariable")->FirstChildElement("SEQUENCE")->FirstChildElement("variableSpecification")->FirstChildElement("name")->FirstChildElement("domain-specific");

    boolean attribute;
    if (readElement->QueryBoolAttribute("specificationWithResult", &attribute) != XML_SUCCESS) {
        Serial.println("Could not obtain the attribute");
        return;
    };
    Serial.println(attribute);


    XMLElement* invokeIdElement = root->FirstChildElement("invokeID");
    const char* invokeID = invokeIdElement->GetText();
    Serial.println(invokeID);

    XMLElement* domainIdElement = domain_specific->FirstChildElement("domainId");
    const char* domainId = domainIdElement->GetText();
    Serial.println(domainId);

    XMLElement* itemIdElement = domain_specific->FirstChildElement("itemId");
    const char* itemId = itemIdElement->GetText();
    Serial.println(itemId);
    

    Serial.println();
    delay(1000);
}
