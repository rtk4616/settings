#include "wifiwidgets.h"

#include <QVBoxLayout>
#include <QMessageBox>

#include "cmessagebox.h"
#include "wifiutil.h"


#ifdef DEVICE_EVB
int wifi_button_width = 145;
int wifi_button_height = 45;
#else
int wifi_button_width = 90;
int wifi_button_height = 30;
#endif

bool lanOldState = false;
bool lanNewState = false;

WifiWidgets::WifiWidgets(QWidget *parent):BaseWidget(parent)
{
    // Set background color.
    setObjectName("WifiWidgets");
    setStyleSheet("WifiWidgets{background:rgb(33,36,43)}");

    initLayout();
    initData();
    initConnection();
}

void WifiWidgets::initData()
{
    // Saved wifi interface class for wifiManager(update ui).
    wifiWid = this;

    m_netManager = WpaManager::getInstance(this);

    m_wifiSwitch->getSwitchButton()->setChecked(WifiUtil::is_supplicant_running());
    m_hostAPSwitch->getSwitchButton()->setChecked(WifiUtil::is_hostapd_running());

    // Check LAN connection.
    m_workTimer = new QTimer(this);
    m_workTimer->setSingleShot(false);
#ifdef DEVICE_EVB
    connect(m_workTimer, SIGNAL(timeout()), this, SLOT(slot_checkLanConnection()));
    m_workTimer->start(5000);
#endif

    WifiUtil::creat_supplicant_file();
    WifiUtil::creat_hostapd_file("RK_HOSTAPD_TEST", "12345678");
}

void WifiWidgets::initLayout()
{
    m_wifiSwitch = new SwitchWidget(this,tr("OPEN WIFI"));

    // Include 'scan result' and 'current status'.
    m_tab = new QTabWidget(this);

    m_tab->setStyleSheet("QTabWidget{background:rgb(33,36,43)}");
    m_tabCurrentStatus = new TabCurrentStatus(this);
    m_tabCurrentStatus->connectButton->setText(tr("Connect"));
    m_tabCurrentStatus->disconnectButton->setText(tr("Disconnect"));

    m_tabCurrentStatus->lastMessageLabel->setText(tr("Last message:"));
    m_tabCurrentStatus->authenticationLabel->setText(tr("Authentication:"));
    m_tabCurrentStatus->encryptionLabel->setText(tr("EncryptionLabel:"));
    m_tabCurrentStatus->ssidLabel->setText(tr("SSID:"));
    m_tabCurrentStatus->bssidLabel->setText(tr("BSSID:"));
    m_tabCurrentStatus->ipAddressLabel->setText(tr("IP Address:"));

    m_tabScanResult = new TabScanResult(this);
    m_tabScanResult->scanButton->setText(tr("Scan"));
    m_tabNetworkManager = new TabNetworkManager(this);
    m_tabNetworkManager ->removeButton->setText(tr("remove"));
    m_tabNetworkManager ->editButton->setText(tr("edit"));
    m_tab->addTab(m_tabScanResult,tr("Scan Result"));
    m_tab->addTab(m_tabCurrentStatus,tr("Current Status"));
    m_tab->addTab(m_tabNetworkManager,tr("Network Manager"));
    m_tab->setCurrentWidget(m_tabScanResult);

    // Related to HOST AP.
    m_hostAPSwitch = new SwitchWidget(this,tr("START HOSTAP"));

    QFrame *bottomLine = new QFrame(this);
    bottomLine->setFixedHeight(2);
    bottomLine->setStyleSheet("QFrame{border:1px solid rgb(100,100,100,255);}");
    bottomLine->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    apNameLabel = new QLabel(this);
    apNameLabel->setText(tr("HotSpot Name:"));

    m_hostAPName = new QLineEdit("RK_HOSTAP_TEST",this);
    m_hostAPName->setFixedSize(wifi_button_width + 200,wifi_button_height);
    m_hostAPName->setAlignment(Qt::AlignRight);

    QHBoxLayout *apNameLayout = new QHBoxLayout;
    apNameLayout->addWidget(apNameLabel);
    apNameLayout->addWidget(m_hostAPName);

    apPasswordLabel= new QLabel(this);
    apPasswordLabel->setText(tr("HotSpot Password:"));

    m_hostAPPassword = new QLineEdit("987654321",this);
    m_hostAPPassword->setFixedSize(wifi_button_width + 200,wifi_button_height);
    m_hostAPPassword->setAlignment(Qt::AlignRight);

    QHBoxLayout *apPasswordLayout = new QHBoxLayout;
    apPasswordLayout->addWidget(apPasswordLabel);
    apPasswordLayout->addWidget(m_hostAPPassword);


    QHBoxLayout *layoutStatus = new QHBoxLayout;
    statusLabel = new QLabel(this);
    statusLabel->setText(tr("Status:"));
    textStatus = new QLabel(this);
    textStatus->setAlignment(Qt::AlignRight);

    layoutStatus->addWidget(statusLabel,1);
    layoutStatus->addWidget(textStatus,2);

    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addSpacing(20);
    vmainlyout->addWidget(m_wifiSwitch);
    vmainlyout->addLayout(layoutStatus);
    vmainlyout->addWidget(m_tab);

    vmainlyout->addWidget(bottomLine);
    vmainlyout->addLayout(apNameLayout);
    vmainlyout->addLayout(apPasswordLayout);
    vmainlyout->addWidget(m_hostAPSwitch);

#ifdef DEVICE_EVB
    vmainlyout->addSpacing(300);
#else
    vmainlyout->addSpacing(30);
#endif

    vmainlyout->setContentsMargins(0,0,0,0);
    vmainlyout->setSpacing(10);

    // Set the layout in the middle.
    QHBoxLayout *hmainlyout = new QHBoxLayout;
    hmainlyout->addStretch(1);
    hmainlyout->addLayout(vmainlyout,6);
    hmainlyout->addStretch(1);
    setLayout(hmainlyout);

    showdialog = new netConfigDialog(this);
    editdialog = new netConfigDialog();
}

void WifiWidgets::initConnection()
{
    connect(m_tabCurrentStatus->connectButton,SIGNAL(clicked(bool)),m_netManager,SLOT(connectB()));
    connect(m_tabCurrentStatus->disconnectButton,SIGNAL(clicked(bool)),m_netManager,SLOT(disconnectB()));
    connect(m_tabScanResult->scanButton,SIGNAL(clicked(bool)),m_netManager,SLOT(scan()));
    connect(m_tabNetworkManager->removeButton,SIGNAL(clicked(bool)),this,SLOT(slot_removeListedNetwork()));
    connect(m_tabNetworkManager->editButton,SIGNAL(clicked(bool)),this,SLOT(slot_editListedNetwork()));

    connect(m_tabScanResult->m_table,SIGNAL(cellClicked(int,int)),this,SLOT(slot_showItemDetail(int,int)));
    connect(m_wifiSwitch->getSwitchButton(),SIGNAL(checkStateChanged(bool)),this,SLOT(slot_onWifiToggled(bool)));
    connect(m_hostAPSwitch->getSwitchButton(),SIGNAL(checkStateChanged(bool)),this,SLOT(slot_onHostapdToggled(bool)));

    connect(mainWindow,SIGNAL(retranslateUi()),this,SLOT(retranslateUi()));
}
void WifiWidgets::retranslateUi(){
    m_wifiSwitch->setText(tr("OPEN WIFI"));
    m_hostAPSwitch->setText(tr("START HOSTAP"));

    apNameLabel->setText(tr("HotSpot Name:"));
    apPasswordLabel->setText(tr("HotSpot Password:"));
    statusLabel->setText(tr("Status:"));

    m_tab->setTabText(0,tr("Scan Result"));
    m_tab->setTabText(1,tr("Current Status"));
    m_tab->setTabText(2,tr("Network Manager"));

    m_tabScanResult->scanButton->setText(tr("Scan"));
    m_tabNetworkManager ->removeButton->setText(tr("remove"));
    m_tabNetworkManager ->editButton->setText(tr("edit"));
    m_tabCurrentStatus->connectButton->setText(tr("Connect"));
    m_tabCurrentStatus->disconnectButton->setText(tr("Disconnect"));

    m_tabCurrentStatus->lastMessageLabel->setText(tr("Last message:"));
    m_tabCurrentStatus->authenticationLabel->setText(tr("Authentication:"));
    m_tabCurrentStatus->encryptionLabel->setText(tr("EncryptionLabel:"));
    m_tabCurrentStatus->ssidLabel->setText(tr("SSID:"));
    m_tabCurrentStatus->bssidLabel->setText(tr("BSSID:"));
    m_tabCurrentStatus->ipAddressLabel->setText(tr("IP Address:"));
}

void WifiWidgets::slot_editListedNetwork()
{
    if (m_tabNetworkManager->m_networkList->currentRow() < 0) {
        return;
    }

    QString cmd(m_tabNetworkManager->m_networkList->currentItem()->text());
    int id = -1;

    if (cmd.contains(QRegExp("^\\d+:"))) {
        cmd.truncate(cmd.indexOf(':'));
        id = cmd.toInt();
    }


    if (editdialog == NULL)
        return;

    if (id >= 0){
        editdialog->paramsFromConfig(id);
        editdialog->show();
        editdialog->exec();
    }
}

void WifiWidgets::slot_removeListedNetwork()
{
    if (m_tabNetworkManager->m_networkList->currentRow() < 0) {
        return;
    }
    QString sel(m_tabNetworkManager->m_networkList->currentItem()->text());
    m_netManager->removeNetwork(sel);
}

void WifiWidgets::slot_showItemDetail(int row,int)
{

    if (showdialog == NULL)
        return;
    showdialog->paramsFromScanResults(m_tabScanResult->m_netWorks[row]);
    showdialog->show();
    showdialog->exec();
}

void WifiWidgets::slot_onWifiToggled(bool isChecked)
{
    if(isChecked){
        if(WifiUtil::is_hostapd_running()){
            CMessageBox::showCMessageBox(this,tr("Close hostap first."),tr("Confirm"),tr("Cancel"));
            QTimer::singleShot(10,this,SLOT(setWifiUnchecked()));
        }else{
            wifiStationOpen();
        }
    }else{
        wifiStationClose();
    }
}

void WifiWidgets::slot_onHostapdToggled(bool isChecked){
    if(isChecked){
        if(m_hostAPName->text()== NULL || m_hostAPPassword->text().size() < 8){
            QMessageBox::warning(this,tr("Warning"),tr("Name can't be null,and password can't be less 8!"),QMessageBox::Ok);
            QTimer::singleShot(10,this,SLOT(setHostapdUnchcked()));
        }else if(WifiUtil::is_supplicant_running()){
            CMessageBox::showCMessageBox(this,tr("Close wifi first."),tr("Confirm"),tr("Cancel"));
            QTimer::singleShot(10,this,SLOT(setHostapdUnchcked()));
        }else {
            WifiUtil::creat_hostapd_file(m_hostAPName->text().toLatin1().data(),m_hostAPPassword->text().toLatin1().data());
            WifiUtil::wifi_start_hostapd();
        }
    }else{
        WifiUtil::wifi_stop_hostapd();
    }
}

void WifiWidgets::setHostapdUnchcked()
{
    m_hostAPSwitch->getSwitchButton()->setChecked(false);
}

void WifiWidgets::setWifiUnchecked()
{
    m_wifiSwitch->getSwitchButton()->setChecked(false);
}

void WifiWidgets::wifiStationOpen()
{
    // Start wap_supplicant service.
    NetThread *thread = new NetThread;
    thread->start();

    m_netManager->openCtrlConnection("wlan0");
}

void WifiWidgets::wifiStationClose()
{
    WifiUtil::wifi_stop_supplicant();
    m_tabScanResult->clearTable();
}

void WifiWidgets::wifiHostapdOpen()
{
    WifiUtil::wifi_start_hostapd();
}

void WifiWidgets::wifiHostapdClose()
{
    WifiUtil::wifi_stop_hostapd();
}

void WifiWidgets::getIPAddress()
{
    WifiUtil::getIPAddress();
}

void WifiWidgets::slot_checkLanConnection()
{
    char cmdbuf[1024] = {0};
    char cmdresult[1024] = {0};

    sprintf(cmdbuf, "cat /sys/class/net/eth0/carrier");
    FILE *pp = popen(cmdbuf, "r");
    if (!pp) {
        qDebug("Running cmdline failed:cat /sys/class/net/eth0/carrier\n");
        return;
    }
    fgets(cmdresult, sizeof(cmdresult), pp);
    pclose(pp);

    if(strstr(cmdresult, "1"))
    {
        lanNewState = true;
    }else if(strstr(cmdresult, "0")){
        lanNewState = false;
    }else{
        WifiUtil::console_run("ifconfig eth0 up");
    }
    if(lanOldState != lanNewState){
        if(lanNewState){
            //LanConnected
            WifiUtil::lanStateChanhe(lanNewState);
        }else{
            //LanDisconnected
            WifiUtil::lanStateChanhe(lanNewState);
        }
        lanOldState = lanNewState;
    }
}


TabScanResult::TabScanResult(QWidget *parent):BaseWidget(parent)
{
    vmainlyout = new QVBoxLayout;

    m_table = new WlanListTable(this);

    lyout1 = new QHBoxLayout;
    scanButton = new QPushButton(this);

    scanButton->setFixedSize(wifi_button_width,wifi_button_height);

    lyout1->addStretch(0);
    lyout1->addWidget(scanButton);
    lyout1->addStretch(0);

    vmainlyout->addSpacing(5);
    vmainlyout->addWidget(m_table);
    vmainlyout->addSpacing(10);
    vmainlyout->addLayout(lyout1);
    vmainlyout->setContentsMargins(10,10,5,5);
    vmainlyout->setSpacing(5);
    setLayout(vmainlyout);
}
void TabScanResult::setText(QString text){
    scanButton->setText(text);
}

void TabScanResult::clearTable()
{
    int iLen = m_table->rowCount();
    for(int i=0;i<iLen;i++)
    {
        m_table->removeRow(0);
    }
    m_table->clear();
}

void TabScanResult::insertIntoTable(QString ssid, QString bssid, QString siganl, QString flags)
{
    int rowCount = m_table->rowCount();
    m_table->insertRow(rowCount);

    m_table->setItem(rowCount,0,new QTableWidgetItem(ssid));
    m_table->setItem(rowCount,1,new QTableWidgetItem(bssid));
    m_table->item(rowCount,0)->setTextAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    m_table->item(rowCount,1)->setTextAlignment(Qt::AlignVCenter|Qt::AlignLeft);

    int siganlValue = siganl.toInt();
    QTableWidgetItem *siganlItem;
    if(siganlValue>=(-55)){
        siganlItem = new QTableWidgetItem(QIcon(":/image/setting/ic_wifi_signal_4_dark.png"),NULL);
    }else if(siganlValue>=(-70)){
        siganlItem = new QTableWidgetItem(QIcon(":/image/setting/ic_wifi_signal_3_dark.png"),NULL);
    }else if(siganlValue>=(-85)){
        siganlItem = new QTableWidgetItem(QIcon(":/image/setting/ic_wifi_signal_2_dark.png"),NULL);
    }else{
        siganlItem = new QTableWidgetItem(QIcon(":/image/setting/ic_wifi_signal_1_dark.png"),NULL);
    }
    m_table->setItem(rowCount,2,siganlItem);
    m_table->item(rowCount,2)->setData(Qt::DisplayRole,siganlValue);


    QString auth;
    if (flags.indexOf("[WPA2-EAP") >= 0)
        auth = "WPA2_EAP";
    else if (flags.indexOf("[WPA-EAP") >= 0)
        auth = "WPA_EAP";
    else if (flags.indexOf("[WPA2-PSK") >= 0)
        auth = "WPA2_PSK";
    else if (flags.indexOf("[WPA-PSK") >= 0)
        auth = "WPA_PSK";
    else
        auth = "OPEN";

    if(auth.compare("OPEN")==0)
    {
        m_table->setItem(rowCount,3,new QTableWidgetItem(QIcon(":/image/setting/ic_wifi_unlocked.png"),auth));
    }else{
        m_table->setItem(rowCount,3,new QTableWidgetItem(QIcon(":/image/setting/ic_wifi_locked.png"),auth));
    }

    // Save data.
    m_netWorks[rowCount].ssid = ssid;
    m_netWorks[rowCount].bssid = bssid;
    m_netWorks[rowCount].signal = siganl;
    m_netWorks[rowCount].flags = flags;
}

TabNetworkManager::TabNetworkManager(QWidget *parent):BaseWidget(parent)
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    m_networkList = new NetworkList(this);
    QHBoxLayout *lyout1 = new QHBoxLayout;

    removeButton = new QPushButton(this);    
    removeButton->setFixedSize(wifi_button_width,wifi_button_height);

    editButton = new QPushButton(this);    
    editButton->setFixedSize(wifi_button_width,wifi_button_height);

    lyout1->addStretch(0);
    lyout1->addWidget(removeButton);
    lyout1->addSpacing(30);
    lyout1->addWidget(editButton);
    lyout1->addStretch(0);

    vmainlyout->addWidget(m_networkList);
    vmainlyout->addSpacing(15);
    vmainlyout->addLayout(lyout1);
    vmainlyout->setContentsMargins(10,10,5,5);
    vmainlyout->setSpacing(5);
    setLayout(vmainlyout);
}

void NetThread::run()
{
    WifiUtil::wifi_start_supplicant();
}

WifiWidgets::~WifiWidgets()
{
}