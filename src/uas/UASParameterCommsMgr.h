#ifndef UASPARAMETERCOMMSMGR_H
#define UASPARAMETERCOMMSMGR_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QVariant>
#include <QVector>

class UASInterface;
class UASParameterDataModel;



class UASParameterCommsMgr : public QObject
{
    Q_OBJECT


public:
    explicit UASParameterCommsMgr(QObject *parent = 0);
    UASParameterCommsMgr* initWithUAS(UASInterface* model);///< Two-stage constructor

    ~UASParameterCommsMgr();

    typedef enum ParamCommsStatusLevel {
        ParamCommsStatusLevel_OK = 0,
        ParamCommsStatusLevel_Warning = 2,
        ParamCommsStatusLevel_Error = 4,
        ParamCommsStatusLevel_Count
    } ParamCommsStatusLevel_t;


protected:
    /** @brief Activate / deactivate parameter retransmission */
    virtual void setRetransmissionGuardEnabled(bool enabled);

    virtual void setParameterStatusMsg(const QString& msg, ParamCommsStatusLevel_t level=ParamCommsStatusLevel_OK);

    /** @brief Load settings that control eg retransmission timeouts */
    void loadParamCommsSettings();

    /** @brief clear transmissionMissingPackets and transmissionMissingWriteAckPackets */
    void clearRetransmissionLists(int& missingReadCount, int& missingWriteCount );

    void resendReadWriteRequests();
    void resetAfterListReceive();

    void emitPendingParameterCommit(int compId, const QString& key, QVariant& value);

signals:
    void commitPendingParameter(int component, QString parameter, QVariant value);
    void parameterChanged(int component, int parameterIndex, QVariant value);
    void parameterValueConfirmed(int uas, int component,int paramCount, int paramId, QString parameter, QVariant value);

    /** @brief We have received a complete list of all parameters onboard the MAV */
    void parameterListUpToDate();

    void parameterUpdateRequested(int component, const QString& parameter);
    void parameterUpdateRequestedById(int componentId, int paramId);

    /** @brief We updated the parameter status message */
    void parameterStatusMsgUpdated(QString msg, int level);

public slots:
    /** @brief  Iterate through all components, through all pending parameters and send them to UAS */
    virtual void sendPendingParameters(bool copyToPersistent = false);

    /** @brief  Write the current onboard parameters from transient RAM into persistent storage, e.g. EEPROM or harddisk */
    virtual void writeParamsToPersistentStorage();

    /** @brief Write one parameter to the MAV */
    virtual void setParameter(int component, QString parameterName, QVariant value);

    /** @brief Request list of parameters from MAV */
    virtual void requestParameterList();

    /** @brief Check for missing parameters */
    virtual void retransmissionGuardTick();

    /** @brief Request a single parameter update by name */
    virtual void requestParameterUpdate(int component, const QString& parameter);

    /** @brief Request an update of RC parameters */
    virtual void requestRcCalibrationParamsUpdate();

    virtual void receivedParameterUpdate(int uas, int compId, int paramCount, int paramId, QString paramName, QVariant value);

protected:

    UASInterface* mav;   ///< The MAV we're talking to

    UASParameterDataModel* paramDataModel;

    // Communications management
    QVector<bool> receivedParamsList; ///< Successfully received parameters
    QMap<int, QList<int>* > missingReadPackets; ///< Missing packets
    QMap<int, QMap<QString, QVariant>* > missingWriteAckPackets; ///< Missing write ACK packets
    bool transmissionListMode;       ///< Currently requesting list
    QMap<int, bool> transmissionListSizeKnown;  ///< List size initialized?
    bool transmissionActive;         ///< Missing packets, working on list?
    bool persistParamsAfterSend; ///< Copy all parameters to persistent storage after sending
    quint64 transmissionTimeout;     ///< Timeout
    QTimer retransmissionTimer;      ///< Timer handling parameter retransmission
    quint64 lastTimerReset;     ///< Last time the guard timer was reset, to prevent premature firing
    int retransmissionTimeout; ///< Retransmission request timeout, in milliseconds
    int rewriteTimeout; ///< Write request timeout, in milliseconds
    int retransmissionBurstRequestSize; ///< Number of packets requested for retransmission per burst
    quint64 listRecvTimeout;     ///< How long to wait for first parameter list response before re-requesting

    // Status
    QString parameterStatusMsg;

};
    

#endif // UASPARAMETERCOMMSMGR_H