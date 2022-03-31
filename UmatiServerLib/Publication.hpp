/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2020-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Christoph Ruckstetter, Michael Weinig AG
 */

#pragma once

#include <open62541/server.h>

#include <Open62541Cpp/UA_QualifiedName.hpp>
#include <exception>
#include <refl.hpp>
#include <sstream>
#include <bitset>

#include "BindRefl.hpp"
#include "BindableMember.hpp"
#include "NodesMaster.hpp"
#include "Util.hpp"


/* -------------------------------------------------------------------- */

struct TopicCreator {
    std::string prefix;
    std::string publisherId;
    std::string writerGroupName;
    std::string publishedDataSetName;

    auto getMetaDataTopic() {
        return prefix + "/json/metadata/" + publisherId + "/" + writerGroupName + "/" + publishedDataSetName + "_Writer";
    }

    auto getDataTopic() {
        return prefix + "/json/data/" + publisherId + "/" + writerGroupName + "/" + publishedDataSetName + "_Writer";
    }

    auto getWriterName() {
        return publishedDataSetName + "_Writer";
    }

    auto getPublishedDataSetName() {
        return publishedDataSetName;
    }
};

struct MqttConnectionConfig {
    enum ConnectionOptions {
        CLIENT_ID = 0,
        SEND_BUFFER_SIZE = 1,
        USERNAME = 2,
        PASSWORD = 3,
        USE_TSL = 4,
        CA_FILE_PATH = 5,
        BROKER_URL = 6,
        OPTION_MAX
    };

    UA_PubSubConnectionConfig connectionConfig;
    UA_NodeId connectionIdent;

    std::string mMqttClientId;
    UA_UInt32 mMqttSendBufferSize;
    std::string mMqttUsername;
    std::string mMqttPassword;
    std::string mMqttCaFilePath;
    std::string mMqttBrokerUrl;

    std::bitset<ConnectionOptions::OPTION_MAX> options = 0;

    MqttConnectionConfig() {
        UA_NodeId_init(&connectionIdent);
        memset(&connectionConfig, 0, sizeof(UA_PubSubConnectionConfig));
    };


    MqttConnectionConfig& setClientId(const std::string &clientId) {
        options[ConnectionOptions::CLIENT_ID] = true;
        mMqttClientId = clientId;
        return *this;
    }

    MqttConnectionConfig& setSendBufferSize(const UA_UInt32 sendBufferSize) {
        options[ConnectionOptions::SEND_BUFFER_SIZE] = true;
        mMqttSendBufferSize = sendBufferSize;
        return *this;
    }

    MqttConnectionConfig& setUsername(const std::string &username) {
        options[ConnectionOptions::USERNAME] = true;
        mMqttUsername = username;
        return *this;
    }

    MqttConnectionConfig& setPassword(const std::string &password) {
        options[ConnectionOptions::PASSWORD] = true;
        mMqttPassword = password;
        return *this;
    }

    MqttConnectionConfig& setMqttCaFilePath(const std::string &cafilepath) {
        options[ConnectionOptions::CA_FILE_PATH] = true;
        options[ConnectionOptions::USE_TSL] = true;
        mMqttCaFilePath = cafilepath;
        return *this;
    }

    MqttConnectionConfig& setBrokerUrl(const std::string &brokerUrl) {
        options[ConnectionOptions::BROKER_URL] = true;
        mMqttBrokerUrl = brokerUrl;
        return *this;
    }

    MqttConnectionConfig& buildAndApplyConnectionOptions(UA_Server *server) {
        connectionConfig.name = UA_STRING_ALLOC("MQTT Pub Sub Connection");
        connectionConfig.transportProfileUri = UA_STRING_ALLOC("http://opcfoundation.org/UA-Profile/Transport/pubsub-mqtt");
        connectionConfig.enabled = UA_TRUE;

        UA_NetworkAddressUrlDataType networkAddressUrl = {UA_STRING_NULL , UA_STRING_ALLOC(mMqttBrokerUrl.c_str())};
        UA_Variant_setScalar(&connectionConfig.address, &networkAddressUrl, &UA_TYPES[UA_TYPES_NETWORKADDRESSURLDATATYPE]);
        const size_t connectionOptionsCount = options.count();

        UA_KeyValuePair* connectionOptions = (UA_KeyValuePair*) UA_Array_new(connectionOptionsCount, &UA_TYPES[UA_TYPES_KEYVALUEPAIR]);
        size_t connectionOptionIndex = 0;
        
        UA_String mqttClientId, mqttUsername, mqttPassword, mqttCaFile;
        UA_Boolean mqttUseTLS;
        UA_UInt32 sBs;
        
        if (options[ConnectionOptions::CLIENT_ID]) {
           connectionOptions[connectionOptionIndex].key = UA_QUALIFIEDNAME_ALLOC(0, "mqttClientId");
           mqttClientId = UA_STRING_ALLOC(mMqttClientId.c_str());
           UA_Variant_setScalar(&connectionOptions[connectionOptionIndex++].value, &mqttClientId, &UA_TYPES[UA_TYPES_STRING]);
           connectionConfig.publisherId.string = mqttClientId;
           connectionConfig.publisherIdType = UA_PUBSUB_PUBLISHERID_STRING;
        }

        if (options[ConnectionOptions::SEND_BUFFER_SIZE]) {
            connectionOptions[connectionOptionIndex].key = UA_QUALIFIEDNAME_ALLOC(0, "sendBufferSize");
            sBs = mMqttSendBufferSize;
            UA_Variant_setScalar(&connectionOptions[connectionOptionIndex++].value, &sBs, &UA_TYPES[UA_TYPES_UINT32]);
        }

        if (options[ConnectionOptions::USERNAME]) {
            connectionOptions[connectionOptionIndex].key = UA_QUALIFIEDNAME_ALLOC(0, "mqttUsername");
            mqttUsername = UA_STRING_ALLOC(mMqttUsername.c_str());
            UA_Variant_setScalar(&connectionOptions[connectionOptionIndex++].value, &mqttUsername, &UA_TYPES[UA_TYPES_STRING]);
        }

        if (options[ConnectionOptions::PASSWORD]) {
            connectionOptions[connectionOptionIndex].key = UA_QUALIFIEDNAME_ALLOC(0, "mqttPassword");
            mqttPassword = UA_STRING_ALLOC(mMqttPassword.c_str());
            UA_Variant_setScalar(&connectionOptions[connectionOptionIndex++].value, &mqttPassword, &UA_TYPES[UA_TYPES_STRING]);
        }

        if (options[ConnectionOptions::USE_TSL]) {
            connectionOptions[connectionOptionIndex].key = UA_QUALIFIEDNAME_ALLOC(0, "mqttUseTLS");
            mqttUseTLS = true;
            UA_Variant_setScalar(&connectionOptions[connectionOptionIndex++].value, &mqttUseTLS, &UA_TYPES[UA_TYPES_BOOLEAN]);
        }

        if (options[ConnectionOptions::CA_FILE_PATH]) {
            connectionOptions[connectionOptionIndex].key = UA_QUALIFIEDNAME_ALLOC(0, "mqttCaFilePath");
            mqttCaFile = UA_STRING_ALLOC(mMqttCaFilePath.c_str());
            UA_Variant_setScalar(&connectionOptions[connectionOptionIndex++].value, &mqttCaFile, &UA_TYPES[UA_TYPES_STRING]);    
        }

        connectionConfig.connectionProperties = connectionOptions;
        connectionConfig.connectionPropertiesSize = connectionOptionIndex;
        UA_Server_addPubSubConnection(server, &connectionConfig, &connectionIdent);
        return *this;
    }

    UA_NodeId getConnectionIdent() {
        return connectionIdent;
    }
};

static void
addPublishedDataSet(UA_Server *server, UA_NodeId* publishedDataSetIdent, std::string name, UA_Boolean sendViaWriterGroupTopic=UA_TRUE) {
    UA_NodeId_init(publishedDataSetIdent);
    /* The PublishedDataSetConfig contains all necessary public
    * information for the creation of a new PublishedDataSet */
    UA_PublishedDataSetConfig publishedDataSetConfig;
    memset(&publishedDataSetConfig, 0, sizeof(UA_PublishedDataSetConfig));
    publishedDataSetConfig.publishedDataSetType = UA_PUBSUB_DATASET_PUBLISHEDITEMS;
    publishedDataSetConfig.name = UA_STRING_ALLOC(name.c_str());
    publishedDataSetConfig.sendViaWriterGroupTopic = sendViaWriterGroupTopic;
    /* Create new PublishedDataSet based on the PublishedDataSetConfig. */
    UA_Server_addPublishedDataSet(server, &publishedDataSetConfig, publishedDataSetIdent);
}

/**
 * **DataSetField handling**
 * The DataSetField (DSF) is part of the PDS and describes exactly one published field.
 */
static void
addDataSetField(UA_Server *server, UA_NodeId *publishedDataSetIdent, UA_NodeId toAdd, char* fieldNameAlias) {
    /* Add a field to the previous created PublishedDataSet */
    UA_DataSetFieldConfig dataSetFieldConfig;
    memset(&dataSetFieldConfig, 0, sizeof(UA_DataSetFieldConfig));
    dataSetFieldConfig.dataSetFieldType = UA_PUBSUB_DATASETFIELD_VARIABLE;
    dataSetFieldConfig.field.variable.fieldNameAlias = UA_STRING(fieldNameAlias);
    dataSetFieldConfig.field.variable.promotedField = UA_FALSE;
    dataSetFieldConfig.field.variable.publishParameters.publishedVariable = toAdd;
    dataSetFieldConfig.field.variable.publishParameters.attributeId = UA_ATTRIBUTEID_VALUE;
    UA_Server_addDataSetField(server, *publishedDataSetIdent, &dataSetFieldConfig, NULL);
}

/**
 * **WriterGroup handling**
 * The WriterGroup (WG) is part of the connection and contains the primary configuration
 * parameters for the message creation.
 */
static UA_StatusCode
addWriterGroup(UA_Server *server, char *topic, int interval, UA_NodeId* writerGroupIdent, UA_NodeId* connectionIdent) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    /* Now we create a new WriterGroupConfig and add the group to the existing PubSubConnection. */
    UA_WriterGroupConfig writerGroupConfig;
    memset(&writerGroupConfig, 0, sizeof(UA_WriterGroupConfig));
    writerGroupConfig.name = UA_STRING(const_cast<char*>("Demo WriterGroup"));
    writerGroupConfig.publishingInterval = interval;
    writerGroupConfig.enabled = UA_FALSE;
    writerGroupConfig.writerGroupId = 100;
    writerGroupConfig.maxEncapsulatedDataSetMessageCount = 100;

    /* decide whether to use JSON or UADP encoding*/
    UA_JsonWriterGroupMessageDataType *Json_writerGroupMessage;
    
    {
        writerGroupConfig.encodingMimeType = UA_PUBSUB_ENCODING_JSON;
        writerGroupConfig.messageSettings.encoding             = UA_EXTENSIONOBJECT_DECODED;

        writerGroupConfig.messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_JSONWRITERGROUPMESSAGEDATATYPE];
        /* The configuration flags for the messages are encapsulated inside the
         * message- and transport settings extension objects. These extension
         * objects are defined by the standard. e.g.
         * UadpWriterGroupMessageDataType */
        Json_writerGroupMessage = UA_JsonWriterGroupMessageDataType_new();
        /* Change message settings of writerGroup to send PublisherId,
         * DataSetMessageHeader, SingleDataSetMessage and DataSetClassId in PayloadHeader
         * of NetworkMessage */
        Json_writerGroupMessage->networkMessageContentMask =
            (UA_JsonNetworkMessageContentMask)(UA_JSONNETWORKMESSAGECONTENTMASK_NETWORKMESSAGEHEADER |
            (UA_JsonNetworkMessageContentMask)UA_JSONNETWORKMESSAGECONTENTMASK_DATASETMESSAGEHEADER |
            (UA_JsonNetworkMessageContentMask)UA_JSONNETWORKMESSAGECONTENTMASK_SINGLEDATASETMESSAGE |
            (UA_JsonNetworkMessageContentMask)UA_JSONNETWORKMESSAGECONTENTMASK_PUBLISHERID |
            (UA_JsonNetworkMessageContentMask)UA_JSONNETWORKMESSAGECONTENTMASK_DATASETCLASSID);
        writerGroupConfig.messageSettings.content.decoded.data = Json_writerGroupMessage;
    }

    /* configure the mqtt publish topic */
    UA_BrokerWriterGroupTransportDataType brokerTransportSettings;
    memset(&brokerTransportSettings, 0, sizeof(UA_BrokerWriterGroupTransportDataType));
    /* Assign the Topic at which MQTT publish should happen */
    /*ToDo: Pass the topic as argument from the writer group */
    brokerTransportSettings.queueName = UA_STRING(topic);
    brokerTransportSettings.resourceUri = UA_STRING_NULL;
    brokerTransportSettings.authenticationProfileUri = UA_STRING_NULL;

    /* Choose the QOS Level for MQTT */
    brokerTransportSettings.requestedDeliveryGuarantee = UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT;

    /* Encapsulate config in transportSettings */
    UA_ExtensionObject transportSettings;
    memset(&transportSettings, 0, sizeof(UA_ExtensionObject));
    transportSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    transportSettings.content.decoded.type = &UA_TYPES[UA_TYPES_BROKERWRITERGROUPTRANSPORTDATATYPE];
    transportSettings.content.decoded.data = &brokerTransportSettings;

    writerGroupConfig.transportSettings = transportSettings;
    retval = UA_Server_addWriterGroup(server, *connectionIdent, &writerGroupConfig, writerGroupIdent);

    if (retval == UA_STATUSCODE_GOOD)
        UA_Server_setWriterGroupOperational(server, *writerGroupIdent);

    UA_JsonWriterGroupMessageDataType_delete(Json_writerGroupMessage);

    return retval;
}

/**
 * **DataSetWriter handling**
 * A DataSetWriter (DSW) is the glue between the WG and the PDS. The DSW is
 * linked to exactly one PDS and contains additional information for the
 * message generation.
 */
static void
addDataSetWriter(UA_Server *server, std::string writerName, std::string metadataTopic, std::string dataTopic, UA_NodeId *publishedDataSetIdent, UA_NodeId *writerGroupIdent, UA_NodeId *outDataSetWriterIdent, UA_Boolean reversible) {
    /* We need now a DataSetWriter within the WriterGroup. This means we must
     * create a new DataSetWriterConfig and add call the addWriterGroup function. */
    static UA_UInt16 dataSetWriterId = 0;

    UA_DataSetWriterConfig dataSetWriterConfig;
    memset(&dataSetWriterConfig, 0, sizeof(UA_DataSetWriterConfig));
    dataSetWriterConfig.dataSetWriterId = dataSetWriterId++;
    dataSetWriterConfig.keyFrameCount = 10;

    if(!reversible) {
        dataSetWriterConfig.dataSetFieldContentMask = (UA_DataSetFieldContentMask) (UA_DATASETFIELDCONTENTMASK_RAWDATA);
    }

    UA_JsonDataSetWriterMessageDataType jsonDswMd;
    UA_ExtensionObject messageSettings;
    jsonDswMd.dataSetMessageContentMask = (UA_JsonDataSetMessageContentMask)
    (UA_JSONDATASETMESSAGECONTENTMASK_DATASETWRITERID |
    UA_JSONDATASETMESSAGECONTENTMASK_SEQUENCENUMBER |
    UA_JSONDATASETMESSAGECONTENTMASK_STATUS |
    UA_JSONDATASETMESSAGECONTENTMASK_METADATAVERSION |
    UA_JSONDATASETMESSAGECONTENTMASK_TIMESTAMP);

    messageSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    messageSettings.content.decoded.type = &UA_TYPES[UA_TYPES_JSONDATASETWRITERMESSAGEDATATYPE];
    messageSettings.content.decoded.data = &jsonDswMd;

    dataSetWriterConfig.messageSettings = messageSettings;

    /*TODO: Modify MQTT send to add DataSetWriters broker transport settings */
    /*TODO: Pass the topic as argument from the writer group */
    /*TODO: Publish Metadata to metaDataQueueName */
    /* configure the mqtt publish topic */
    UA_BrokerDataSetWriterTransportDataType *brokerTransportSettings = UA_BrokerDataSetWriterTransportDataType_new();

    /* Assign the Topic at which MQTT publish should happen */
    brokerTransportSettings->queueName = UA_STRING_ALLOC(dataTopic.c_str());
    brokerTransportSettings->resourceUri = UA_STRING_NULL;
    brokerTransportSettings->authenticationProfileUri = UA_STRING_NULL;
    brokerTransportSettings->metaDataQueueName = UA_STRING_ALLOC(metadataTopic.c_str());
    brokerTransportSettings->metaDataUpdateTime = 0;

    /* Choose the QOS Level for MQTT */
    brokerTransportSettings->requestedDeliveryGuarantee = UA_BROKERTRANSPORTQUALITYOFSERVICE_BESTEFFORT;

    /* Encapsulate config in transportSettings */
    UA_ExtensionObject transportSettings;
    memset(&transportSettings, 0, sizeof(UA_ExtensionObject));
    transportSettings.encoding = UA_EXTENSIONOBJECT_DECODED;
    transportSettings.content.decoded.type = &UA_TYPES[UA_TYPES_BROKERDATASETWRITERTRANSPORTDATATYPE];
    transportSettings.content.decoded.data = brokerTransportSettings;

    dataSetWriterConfig.transportSettings = transportSettings;
    dataSetWriterConfig.name = UA_STRING_ALLOC(writerName.c_str());
    auto code = UA_Server_addDataSetWriter(server, *writerGroupIdent, *publishedDataSetIdent,
                               &dataSetWriterConfig, outDataSetWriterIdent);
}

static void removeDataSetWriter(UA_Server *server, UA_NodeId *dsw) {
    UA_Server_removeDataSetWriter(server, *dsw);
}

/*  ------------------------------------------------------------------------ */

/**
 * @brief Instantiate an optional member, which parent is already bind.
 *
 * Checks the node in the type definition and create a variabe/object
 *
 * @param memberPar The member that should be instantiated
 * @param pServer Pointer to UA_Server
 * @param nodesMaster A nodesmaster to manage the binding
 */

class Publisher {
    public:

    Publisher() {};
    
    std::map<std::string, UA_NodeId> m_namesToNodeIds{};

    struct VariantVisitor {
        Publisher *pub;
        UA_Server *server;
        TopicCreator tc;
        UA_NodeId *writerGroupIdent;
        UA_NodeId *publishedDataSetIdent;
        UA_NodeId *nodeId;
        std::string& name;
        bool aggregrate;
        bool reversible;
        bool& addedSomething;

        template<typename T>
        void operator()(T& value) const {
            const auto ref = refl::reflect<T>();

            if constexpr (refl::descriptor::has_attribute<UmatiServerLib::attribute::UaVariableType>(ref)) {
                addDataSetField(server, publishedDataSetIdent, *nodeId, const_cast<char*>(name.c_str()));
                addedSomething = true;           
            } else {
                pub->PublishFields(ref, value, tc, server, writerGroupIdent, aggregrate, reversible);
            }
        }
    };

    template <typename T>
    bool AddFieldsToPublish(const refl::type_descriptor<T> &typeDescriptor, T &instance, TopicCreator tc, UA_Server *server, UA_NodeId *writerGroupIdent, UA_NodeId *publishedDataSetIdent, UA_Boolean aggregate, UA_Boolean reversible) {
        bool addedSomething = false;
        for_each(typeDescriptor.members, [&](const auto member) {
            if constexpr (refl::descriptor::has_attribute<UmatiServerLib::attribute::PlaceholderOptional>(member)) {
                if constexpr (is_same_template<typename decltype(member)::value_type, BindableMemberValue>::value) {
                    UA_NodeId nullNodeid = UA_NODEID_NUMERIC(0, 0);
                    if(UA_NodeId_equal(member(instance).NodeId.NodeId, &nullNodeid)) {}
                    else {
                        addDataSetField(server, publishedDataSetIdent, *member(instance).NodeId.NodeId, const_cast<char*>(member.name.c_str()));
                        addedSomething = true;
                    }
                } else if constexpr (is_same_template<typename decltype(member)::value_type, BindableMember>::value) {
                    auto tc2 = tc;
                    tc2.publishedDataSetName = tc2.publishedDataSetName + "." + std::string(member.name);
                    auto typeDescriptor = refl::reflect(member(instance).value);
                    
                    if constexpr (refl::descriptor::has_attribute<UmatiServerLib::attribute::UaDataType>(typeDescriptor) ||
                                refl::descriptor::has_attribute<UmatiServerLib::attribute::UaVariableType>(typeDescriptor)) {
                        addDataSetField(server, publishedDataSetIdent, *member(instance).NodeId.NodeId, const_cast<char*>(member.name.c_str()));
                        addedSomething = true;
                    } else {
                        PublishFields(typeDescriptor, member(instance).value, tc2, server, writerGroupIdent, aggregate, reversible);
                    }              
                } else { /* Not a BindeableMemberValue and not a BindableMember -> BindeableMemberPlacholder */
                    auto &value = member(instance).value;
                    for (auto &elem: value) {
                        auto nodeId = elem.NodeId.NodeId;
                        UA_QualifiedName qualifiedName;
                        UA_QualifiedName_init(&qualifiedName);
                        UA_Server_readBrowseName(server, *nodeId, &qualifiedName);
                        std::string name((char*)qualifiedName.name.data, qualifiedName.name.length);
                        auto tc2 = tc;
                        tc2.publishedDataSetName = tc2.publishedDataSetName + "." + name;

                        if constexpr (is_same_template<typeof(elem.value), std::variant>::value) {
                            std::visit(VariantVisitor{this, server, tc2, writerGroupIdent, publishedDataSetIdent, elem.NodeId.NodeId, name, aggregate, reversible, addedSomething}, elem.value);
                        } else {
                            if (refl::descriptor::has_attribute<UmatiServerLib::attribute::UaVariableType>(refl::reflect(elem.value)) ||
                                refl::descriptor::has_attribute<UmatiServerLib::attribute::UaVariableType>(refl::reflect(elem.value))) {
                                addDataSetField(server, publishedDataSetIdent, *elem.NodeId.NodeId, const_cast<char*>(name.c_str()));
                                addedSomething = true;
                            } else {
                                PublishFields(refl::reflect(elem.value), elem.value, tc2, server, writerGroupIdent, aggregate, reversible);
                            } 
                        }
                    }
                }
            } else {
                if constexpr (is_same_template<typename decltype(member)::value_type, BindableMemberValue>::value) {
                    std::cout << member.name.c_str() << '\n';
                    auto x = *member(instance).NodeId.NodeId;
                    std::cout << member.name.c_str() << '\n';
                    addDataSetField(server, publishedDataSetIdent, *member(instance).NodeId.NodeId, const_cast<char*>(member.name.c_str()));
                    addedSomething = true;
                } else if constexpr (is_same_template<typename decltype(member)::value_type, BindableMember>::value) {
                    auto tc2 = tc;
                    tc2.publishedDataSetName = tc2.publishedDataSetName + "." + std::string(member.name);
                    auto typeDescriptor = refl::reflect(member(instance).value);
                    if constexpr (refl::descriptor::has_attribute<UmatiServerLib::attribute::UaDataType>(typeDescriptor) ||
                                refl::descriptor::has_attribute<UmatiServerLib::attribute::UaVariableType>(typeDescriptor)) {
                        addDataSetField(server, publishedDataSetIdent, *member(instance).NodeId.NodeId, const_cast<char*>(member.name.c_str()));
                        addedSomething = true;
                    } else {
                        PublishFields(typeDescriptor, member(instance).value, tc2, server, writerGroupIdent, aggregate, reversible);
                    }  
                } else { /* Not a BindeableMemberValue and not a BindableMember -> BindeableMemberPlacholder */
                    auto &value = member(instance).value;
                    for (auto &elem: value) {
                        auto nodeId = elem.NodeId.NodeId;
                        UA_QualifiedName qualifiedName;
                        UA_QualifiedName_init(&qualifiedName);
                        UA_Server_readBrowseName(server, *nodeId, &qualifiedName);
                        std::string s((char*)qualifiedName.name.data, qualifiedName.name.length);
                        auto tc2 = tc;
                        tc2.publishedDataSetName = tc2.publishedDataSetName + "." + s;
                        // PublishFields(refl::reflect(elem.value), elem.value, tc2, server, writerGroupIdent);

                        if constexpr (refl::descriptor::has_attribute<UmatiServerLib::attribute::UaVariableType>(refl::reflect(elem.value))) {
                            addDataSetField(server, publishedDataSetIdent, *elem.NodeId.NodeId, const_cast<char*>(elem.name.c_str()));
                            addedSomething = true;
                        } else {
                            PublishFields(refl::reflect(elem.value), elem.value, tc2, server, writerGroupIdent, aggregate, reversible);
                        } 
                    }
                }
            }
        });
        return addedSomething;
    }

    template <typename T>
    void PublishFields(const refl::type_descriptor<T> &typeDescriptor, T &instance, TopicCreator tc, UA_Server *server, UA_NodeId *writerGroupIdent, UA_Boolean aggregate, UA_Boolean reversible) {
        std::cout << "TypeName: " << typeDescriptor.name << '\n';
        
        UA_NodeId* publishedDataSetIdent = UA_NodeId_new();
        UA_NodeId* dataSetWriterIdent = UA_NodeId_new();
        addPublishedDataSet(server, publishedDataSetIdent, tc.publishedDataSetName, aggregate);

        bool addedSomething = AddFieldsToPublish(typeDescriptor, instance, tc, server, writerGroupIdent, publishedDataSetIdent, aggregate, reversible);
        
        if constexpr (refl::descriptor::has_attribute<Bases>(refl::reflect<T>())) {
            constexpr auto bases = refl::descriptor::get_attribute<Bases>(refl::reflect<T>());
            if constexpr (bases.descriptors.size > 0) {
            refl::util::for_each(bases.descriptors, [&](auto baseDescriptor) {
                    addedSomething |= AddFieldsToPublish(baseDescriptor, static_cast<typename decltype(baseDescriptor)::type &>(instance), tc, server, writerGroupIdent, publishedDataSetIdent, aggregate, reversible);
                });
            }
        }
        if (addedSomething) {
            addDataSetWriter(server, tc.getWriterName() , tc.getMetaDataTopic(), tc.getDataTopic(), publishedDataSetIdent, writerGroupIdent, dataSetWriterIdent, reversible);
            m_namesToNodeIds.insert({tc.getWriterName(), *dataSetWriterIdent});
        } else {
            UA_NodeId_delete(publishedDataSetIdent);
            UA_NodeId_delete(dataSetWriterIdent);
        }
    }

    template <typename T>
    UA_NodeId Publish(T &instance, UA_Server *pServer, UA_NodeId *connectionIdent, NodesMaster &nodesMaster, std::string prefix, std::string publisherId, std::string writerGroupBaseName, int intervalms, UA_Boolean aggregate=UA_TRUE, UA_Boolean reversible=UA_TRUE) {
        constexpr auto typeDescriptor = refl::reflect<T>();
        UA_NodeId *writerGroupIdent = UA_NodeId_new();
        std::string writerGroupName = writerGroupBaseName + "_WriterGroup"; 
        std::string topic = prefix + "/json/data/" + publisherId + "/" + writerGroupName;  
        auto retval = addWriterGroup(pServer, const_cast<char*>(topic.c_str()), intervalms, writerGroupIdent, connectionIdent);
        if (retval != UA_STATUSCODE_GOOD) {
            std::cout << "Error adding WriterGroup " << UA_StatusCode_name(retval) << "\n";
            return UA_NODEID_NUMERIC(0,0);
        }
        TopicCreator tc{prefix, publisherId, writerGroupName, "_"};

        m_namesToNodeIds.insert({writerGroupName, *writerGroupIdent});

        PublishFields(typeDescriptor, instance, tc, pServer, writerGroupIdent, aggregate, reversible);
        return *writerGroupIdent;
    }
};
