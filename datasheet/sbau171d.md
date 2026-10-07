User's Guide
SBAU171D–May2010–RevisedJanuary2016
ADS1298ECG-FE/ADS1198ECG-FE
This user's guide describes the characteristics, operation, and use of the ADS1298ECG-FE and
ADS1198ECG-FE. The ADS1298ECG-FE and ADS1198ECG-FE are evaluation modules for the
ADS1298, an eight-channel, 24-bit, and ADS1198, an eight-channel, 16-bit, analog-to digital converter
(ADC). Both devices provide low-power, integrated analog front-end (AFE) designs for patient monitoring
and portable and high-end electrocardiogram (ECG) and electroencephalogram (EEG) applications. This
user'sguideincludesacompletecircuit description,schematicdiagram,andbillofmaterials.
ThefollowingrelateddocumentsareavailablethroughtheTexasInstrumentswebsiteatwww.ti.com.
Device LiteratureNumber
ADS1298 SBAS459
ADS1198 SBAS471
Contents
1 ADS1298ECG-FE/ADS1198ECG-FEOverview......................................................................... 3
1.1 ImportantDisclaimerInformation................................................................................. 3
1.2 Introduction.......................................................................................................... 4
1.3 SupportedFeatures................................................................................................ 4
1.4 FeaturesNotSupportedinCurrentVersion..................................................................... 4
1.5 ADS1x98ECG-FEHardware...................................................................................... 5
1.6 MinimumSystemRequirementsforADS1x98ECG-FEEvaluationSoftware............................... 5
2 QuickStart.................................................................................................................... 6
2.1 DefaultJumper/SwitchConfiguration............................................................................ 6
2.2 ADS1x98ECG-FEOperation...................................................................................... 7
3 UsingtheADS1298ECG-FESoftware.................................................................................... 8
3.1 ApplicationUserMenu............................................................................................. 9
3.2 Top-LevelApplicationControls ................................................................................... 9
3.3 AboutTab.......................................................................................................... 10
3.4 ADCRegisterTab................................................................................................. 11
3.5 AnalysisTab....................................................................................................... 19
3.6 SaveTab........................................................................................................... 27
4 ADS1x98ECG-FEInputSignals.......................................................................................... 29
4.1 InputShortTesting................................................................................................ 29
4.2 InternalTestSignalsInput ...................................................................................... 29
4.3 TemperatureSensor.............................................................................................. 30
4.4 NormalElectrodeInput .......................................................................................... 31
4.5 MV Input,RLDMeasurement,RLDPositiveElectrodeDriver,andRLDNegativeElectrodeDriver 31
DD
4.6 LeadDerivation.................................................................................................... 32
4.7 WilsonCenterTerminal(WCT).................................................................................. 32
4.8 RightLegDrive.................................................................................................... 32
4.9 PACEDetection................................................................................................... 33
5 ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails.............................................................. 35
5.1 JumperDescription ............................................................................................... 36
5.2 PowerSupply...................................................................................................... 37
5.3 Clock................................................................................................................ 38
5.4 Reference.......................................................................................................... 38
5.5 AnalogOutputSignals............................................................................................ 39
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 1
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com
5.6 DigitalSignals...................................................................................................... 39
5.7 AnalogInputSignals.............................................................................................. 39
AppendixA Schematics,BOM,Layout,andECGCableDetails.......................................................... 41
AppendixB ExternalOptionalHardware...................................................................................... 47
AppendixC SoftwareInstallation.............................................................................................. 50
ListofFigures
1 ADS1x98ECG-FEKit........................................................................................................ 5
2 ADS1x98ECG-FEDefaultJumperLocations............................................................................ 6
3 SoftwareStartScreen/AboutTab ......................................................................................... 8
4 UserMenu-FileItem....................................................................................................... 9
5 UserMenu-HelpItem...................................................................................................... 9
6 TopLevelControls .......................................................................................................... 9
7 Lead-OffStatusRegistersDisplayWindow ............................................................................ 10
8 ChannelRegistersTab.................................................................................................... 11
9 InternalReferenceandBufferConnections............................................................................ 12
10 Lead-OffExcitationOptions............................................................................................... 13
11 InputMultiplexerforaSingleChannel.................................................................................. 13
12 LOFFandRLDTab........................................................................................................ 14
13 LOFF_STATPandLOFF_STATNComparators....................................................................... 15
14 GPIOandOTHERRegisterTab......................................................................................... 16
15 WilsonCentralandAugmentedLeadRoutingDiagrams............................................................. 17
16 DeviceRegistersSettings................................................................................................. 18
17 ScopeToolFeatures....................................................................................................... 19
18 ScopeAnalysisTab(NoiseLevelsforEachChannelShown)....................................................... 19
19 ZoomToolOptions......................................................................................................... 20
20 HistogramBinsfor12-LeadECGSignal................................................................................ 21
21 StatisticsfortheSignalAmplitudeofEightECGChannels.......................................................... 21
22 FFTGraphofNormalElectrodeConfiguration......................................................................... 22
23 ACAnalysisParameters:WindowingOptions ......................................................................... 22
24 FFTAnalysis:InputShortCondition..................................................................................... 23
25 ChangingtheUser-DefinedDynamicRangeforChannel1.......................................................... 23
26 FFTPlotUsingZoomTool................................................................................................ 24
27 ECGDisplayTabShowingLEADI-IIIandAugmentedLeads....................................................... 25
28 ECGSignalZoomFeatureforSixLeads............................................................................... 26
29 ECGSignalZoomFeatureforLead1................................................................................... 26
30 SaveTab.................................................................................................................... 28
31 ExampleofInternalTestSignalsViewedontheECGDisplayTab................................................. 29
32 InternalTemperatureSensor............................................................................................. 30
33 Eight-ChannelReadofInternalTemperature.......................................................................... 30
34 NormalElectrodeECGConnectioninECGDisplayTab............................................................. 31
35 DigitizationofPACESignalUsingADS1298........................................................................... 34
36 ADS1298ECG-FEFront-EndBlockDiagram........................................................................... 35
37 FlukeSimulatorConfiguration............................................................................................ 40
38 TopComponentPlacement............................................................................................... 44
39 Top Layer ................................................................................................................... 44
40 BottomComponentPlacement........................................................................................... 44
41 Bottom Layer................................................................................................................ 44
42 InternalGroundPlane(Layer2).......................................................................................... 44
2 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1298ECG-FE/ADS1198ECG-FEOverview
43 InternalPowerPlane(Layer3)........................................................................................... 44
44 ECGCableSchematic..................................................................................................... 46
45 15-Pin,ShieldedConnectorfromBiometricCables................................................................... 47
46 15-Pin,TwistedWireCabletoBananaJacks.......................................................................... 47
47 15-Pin,TwistedWireCable............................................................................................... 47
48 CardiosimECGSimulatorTool........................................................................................... 48
49 RecommendedPowerSupplyforADS1x98ECG-FE.................................................................. 49
50 InitializationofADS1x98ECG-FE........................................................................................ 50
51 LicenseAgreement........................................................................................................ 50
52 InstallationProcess........................................................................................................ 50
53 CompletionofADS1x98ECG-FESoftwareInstallation................................................................ 50
ListofTables
1 ADS1x98ECG-FEDefaultJumper/SwitchConfiguration............................................................... 7
2 ADS1298LeadMeasurements........................................................................................... 32
3 DerivedLeadCalculations................................................................................................ 32
4 RLDJumperOptions...................................................................................................... 33
5 ADS1x98ECG-FEDefaultJumper/SwitchConfiguration............................................................. 36
6 Power-SupplyTestPoints................................................................................................. 37
7 AnalogSupplyConfigurations(AVDD/AVSS).......................................................................... 38
8 DigitalSupplyConfigurations(DVDD/DGND).......................................................................... 38
9 CLKJumperOptions....................................................................................................... 38
10 ExternalReferenceJumperOptions..................................................................................... 39
11 Test Signals................................................................................................................. 39
12 SerialInterfacePinout..................................................................................................... 39
13 BillofMaterials:ADS1x98ECG-FE ..................................................................................... 42
Trademarks
PentiumIII,CeleronareregisteredtrademarksofIntelCorporation.
Microsoft,WindowsareregisteredtrademarksofMicrosoftCorporation.
SPI isatrademarkofMotorolaInc.
Allothertrademarksaretheproperty oftheirrespectiveowners.
1 ADS1298ECG-FE/ADS1198ECG-FE Overview
1.1 Important Disclaimer Information
CAUTION
NOTICE: The ADS1298ECG-FE and ADS1198ECG-FE are intended for
feasibility and evaluation testing only in laboratory and development
environments. This product is not for diagnostic use. This product is not for use
withadefibrillator.
TheADS1298ECG-FE/ADS1198ECG-FEistobeusedonlyundertheseconditions:
• TheADS1298ECG-FE/ADS1198ECG-FEisintendedonlyfor electricalevaluationofthefeaturesof
theADS1298deviceinalaboratory,simulation,ordevelopmentenvironment.
• TheADS1298ECG-FE/ADS1198ECG-FEis notintendedfor directinterfacewithapatient,patient
diagnostics,orwithadefibrillator.
• TheADS1298ECG-FE/ADS1198ECG-FEisintendedfor developmentpurposes ONLY.It isnot
intendedtobeusedasallorpartofanendequipmentapplication.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 3
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1298ECG-FE/ADS1198ECG-FEOverview www.ti.com
• TheADS1298ECG-FE/ADS1198ECG-FEshouldbeusedonlybyqualifiedengineersandtechnicians
whoarefamiliarwiththerisksassociatedwithhandlingelectricalandmechanicalcomponents,
systems,andsubsystems.
• Theuserisresponsibleforthesafetyofthemselves,fellowemployeesandcontractors,andco-
workerswhenusingorhandlingtheADS1298ECG-FE/ADS1198ECG-FE. Furthermore,theuseris
fullyresponsibleforthecontactinterfacebetweenthehumanbodyandelectronics;consequently,the
userisresponsibleforpreventingelectricalhazardssuchasshock,electrostaticdischarge,and
| electricaloverstressofelectriccircuit |     |     | components. |
| ------------------------------------- | --- | --- | ----------- |
1.2 Introduction
TheADS1x98ECG-FEisintendedforevaluatingtheADS1298 andADS1198for ECGandEEG
applications.Thedigital SPI™controlinterfaceisprovidedbytheMMB0ModularEVMmotherboardthat
connectstotheADS1x98ECGFEevaluationboard.TheADS1x98ECG-FE(seeFigure1)isNOT a
referencedesignforECGandEEGapplications; rather,itspurposeistoexpediteevaluationandsystem
development.TheoutputoftheADS1298yieldsaraw,unfilteredECGsignal.
TheMMB0motherboardallowstheADS1x98tobeconnectedtothecomputerviaanavailableUSBport.
ThismanualshowshowtousetheMMB0aspart oftheADS1x98ECG-FE, butdoesnotprovidetechnical
detailsabouttheMMB0itself.
ThisdocumentcoverstheoperationoftheADS1x98ECG-FEevaluationsystem. Throughoutthe
document,theabbreviationEVMandthetermevaluationmodulearesynonymouswiththe
ADS1x98ECG-FE.Forclarityofreading,thismanualwillrefer onlytotheADS1298ECG-FEor
ADS1x98ECG-FE,butoperationoftheADS1198ECG-FEisidentical,unlessotherwisenoted.
CAUTION
Many of the components on the ADS1x98ECG-FE are susceptible to damage
by electrostatic discharge (ESD). Customers are advised to observe proper
ESD handling precautions when unpacking and handling the EVM, including
the use of a grounded wrist strap, bootstraps, or mats at an approved ESD
workstation.Anelectrostaticsmockandsafetyglassesshouldalsobeworn.
| 1.3 Supported | Features |     |     |
| ------------- | -------- | --- | --- |
HardwareFeatures:
| • Configurableforbipolarorunipolarsupplyoperation |     |     |     |
| ------------------------------------------------- | --- | --- | --- |
• Configurableforinternalandexternalclockandreferenceviajumpersettings
| • ConfigurableforAC-orDC-coupledinputs  |     |     |        |
| --------------------------------------- | --- | --- | ------ |
| • Configurableforupto12standardECGleads |     |     |        |
| • ExternalRightLegDrive(RLD)Reference(V |     |     | –V )/2 |
CC EE
| • ExternalWilsoncentralvoltage           |     |     |     |
| ---------------------------------------- | --- | --- | --- |
| • EasyconnectivitytopopularECGsimulators |     |     |     |
SoftwareFeatures:
| • Designedtodisplay12leadECGdata |     |     |     |
| -------------------------------- | --- | --- | --- |
• Analysistoolsincludingavirtualoscilloscope,histogram,FFT,andECGdisplay
| • Fileprintingforpost-processingofrawECGdata |     |     |     |
| -------------------------------------------- | --- | --- | --- |
• SetstheADS1298/ADS1198registersettingsviaeasy-to-usegraphicuserinterface(GUI) software
• Post-processingofECGdatausinghigh-pass,low-pass, and50/60Hznotchfilters
| 1.4 Features                 | Not Supported | in Current | Version |
| ---------------------------- | ------------- | ---------- | ------- |
| • Real-timedataprocessing    |               |            |         |
| • AClead-offdetectionfilters |               |            |         |
4 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1298ECG-FE/ADS1198ECG-FEOverview
• QRS detectionalgorithms
• SoftwarePACEdetectionalgorithms
1.5 ADS1x98ECG-FE Hardware
Figure1 showsthehardwareincludedintheADS1x98ECG-FEkit.Contactthefactoryifanycomponentis
missing.Thelatestsoftwareisavailablefor downloadontheTIwebsiteathttp://www.ti.com.
Figure1.ADS1x98ECG-FEKit
Thecompletekitincludesthefollowingitems:
• ADS1x98ECG-FEprintedcircuitboard(PCB)
• MMB0(ModularEVMmotherboard)
1.6 Minimum System Requirements for ADS1x98ECG-FE Evaluation Software
TheminimumsystemrequirementsforusingtheADS1x98ECG-FEsoftwareapplicationare:
• PentiumIII®/Celeron® processor,866MHzorequivalent
• Minimum256MB ofRAM(512MBorgreater recommended)
• USB1.1-compatibleinput
• Harddiskdrivewithatleast200MBfreespace
• Microsoft®Windows® XPoperatingsystemwithSP2orWindows7operatingsystems(WindowsVista
nottested)
• Mouseorotherpointingdevice
• 1280x960minimumdisplayresolution
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 5
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

QuickStart www.ti.com
2 Quick Start
ThissectionprovidesaQuickStartguidetoquicklybeginevaluatingtheEVMusingtheADS1x98ECG-FE
software.
2.1 Default Jumper/Switch Configuration
Figure2 showsthejumpersfoundontheADS1x98ECG-FEEVMandtherespectivefactorydefault
conditionsforeach.
Figure2.ADS1x98ECG-FEDefaultJumper Locations
Table1liststhejumpersandswitchesandthefactorydefaultconditions.
6 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com QuickStart
Table1.ADS1x98ECG-FEDefaultJumper/SwitchConfiguration
Jumper DefaultPosition Description
JP1 Installed RLDfeedback
JP2 Installed1-2 AVDDselectedforbipolarsupplyoperationselected(AVDD=+2.5V)
JP3 HeaderNotInstalled ExternalVrefbuffernotconnected
JP4 Installed EVM+5VprovidedfromJ4(powerheader)
JP5 Open PWDNpincontrolledfromJ5header(pulleduptoDVDD)
HeaderNotInstalled(Pins1-2
JP6toJP14 DC-coupledinputsignals
shortedonPCB)
JP15 Installed2-3 Shielddriveisopen
JP16 Installed WilsonCentralTerminal(WCT)connectedtoINMforCH1andCH4-8
JP17 HeaderNotInstalled ECGshielddrive
JP18 Installed2-3 CLKconnectedtoOSC1
JP19 Installed1-2 OSC1enabled
JP20 Installed2-3 AVSSselectedforbipolarsupplyoperation(AVSS=-2.5V)
JP21 Installed1-2 CSconnectedtoDSPviaJ3.1
JP22 Installed2-3 STARTcomesfromJ3.14
JP23 Installed1-2 CLKSELsetto0(ADS1198usesExtMasterClock(OSC1))
JP24 Installed2-3 DVDDsupply=3.3V
JP25 HeaderNotInstalled Noexternalreferenceselected
Installed1-2(top) WCTconnectedtoCH8-input
JP26
Installed3-4(bottom) ECG_V1connectedtoCH8+input
Installed1-2(top) WCTconnectedtoCH7-input
JP27
Installed3-4(bottom) ECG_V5connectedtoCH7+input
Installed1-2(top) WCTconnectedtoCH6-input
JP28
Installed3-4(bottom) ECG_V3connectedtoCH5+input
Installed1-2(top) WCTconnectedtoCH5-input
JP29
Installed3-4(bottom) ECG_V4connectedtoCH6+input
Installed1-2(top) WCTconnectedtoCH4-input
JP30
Installed3-4(bottom) ECG_V2connectedtoCH4+input
Installed1-2(top) ECG_RAconnectedtoCH3-input
JP31
Installed3-4(bottom) ECG_LLconnectedtoCH3+input
Installed1-2(top) ECG_RAconnectedtoCH2-input
JP32
Installed3-4(bottom) ECG_LAconnectedtoCH2+input
Installed1-2(top) WCTconnectedtoCH1-input
JP33
Installed3-4(bottom) ECG_V6connectedtoCH1+input
2.2 ADS1x98ECG-FE Operation
TopreparetoevaluatetheADS1298withtheADS1298ECG-FE, completethefollowingsteps:
1. VerifythejumpersontheADS1298ECG-FEareasshowninFigure2 (notethat thesesettingsare the
factory-configuredsettingsfortheboard).
2. VerifythatthejumpersontheMMB0motherboardareconfiguredasshownbelow:
• MMB0J13A→Open
• MMB0J13B→Open
• RefertoSectionB.2fordetailsabouttheMMB0power supply.
3. InstalltheADS1298ECG-FEsoftwareusingthelatestsoftwareversion.Thelatestsoftwarecan be
downloadedfromtheADS1298ECG-FEproductpageorADS1198ECG-FEproduct page.Doubleclick
theinstallerandfollowtheinstructiontocompletethesoftwareinstallation.Fordetailedinstallation
informationandscreenshots,seeAppendixC.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 7
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
| 3 Using | the ADS1298ECG-FE | Software |
| ------- | ----------------- | -------- |
TheADS1298ECG-FE softwareprovidescompletecontroloverallthesettingsoftheADS1298.Byusing
theuserinterface(UI),theADS1298controlregisterscanbemanipulatedtoevaluatethevariousoptions
availableonthedevice.Figure3showsthestartingUIscreenofthesoftware. TheUIconsistsofauser
menu(Section3.1),afewtop-levelcontrols(Section3.2), andatabbedinterface, withdifferent functions
availableonthedifferenttabs.Thetabsare:
| •   | About(Section3.3)         |     |
| --- | ------------------------- | --- |
| •   | ADCRegister(Section3.4.3) |     |
| •   | Analysis(Section3.5)      |     |
| •   | Save(Section3.6)          |     |
Figure3.SoftwareStartScreen/AboutTab
Theusercanadjustthesettingswhenthesoftwareisnotacquiringdata.Duringacquisition,allcontrols
aredisabledandsettingsmaynotbechanged.Whenasettingischangedviaacontrol,thesettingsare
immediatelyupdatedonthedeviceandEVM. Settingsinthesoftwarecorrespondtosettingsdescribedin
theADS1298productdatasheet.
8 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3.1 Application User Menu
Theapplicationusermenuislocatedalongthetopoftheapplicationmenu. It consistsoftwoitems: File
andHelp.
FileMenu(seeFigure4)
TheFilemenuprovidesseveraloptions:
• CaptureScreen takesascreencaptureofthecurrent viewoftheapplicationandsavesitasto a
filespecifiedbytheuser.
• SaveConfigurationSettings savesthecurrent statesoftheADS1298controlregistersfor
reloadingatalatertime.Thisfileisdifferent fromthesaveregisteronthe Savetab(see
Section3.6),whichsavesthecurrentregistermaptoatab-delimitedtext file.
• Load ConfigurationSettings loadsapreviouslysaveconfigurationsettingfileandinitializesthe
hardwareandsoftwaretothesettingswithintheconfigurationfile. Theconfigurationfilemustbe a
filesavedpreviouslyfromthisapplicationusingtheSave ConfigurationSettingscommand,nota
filefromtheSavetab.
• Exitclosestheapplication.
HelpMenu(seeFigure5)
TheHelpmenuprovidesthe Aboutoption,whichdisplaythesoftwareandfirmwareversionthatis
currentlybeingused.Pleasehavethisinformationifyouneedtorequest assistanceorhaveaquestion
regardingthesoftwareorhardware.
Figure5.UserMenu-HelpItem
Figure4.UserMenu-FileItem
3.2 Top-Level Application Controls
Severalcontrols/indicatorsarelocatedalongthetopoftheUIscreen(seeFigure6). Thecontrolsand
indicatorsaredescribedbelow.
Figure6.TopLevel Controls
TheDataRateindicatordisplaysthecurrent datarateoftheADS1298.Thedataratecanbeconfiguredin
CONFIG1controlregister(seeSection3.4.2.1).
TheProgressindicatorwilldisplaythecurrent progressofdatatransfertothePCduringacquisition
cycles.
TheSamples/CHcontrolallowsfortheselectionofthenumberofpoints,perchannel, tocollectduringan
acquisitioncycle.Keepinmindthevalueenteredintothiscontrolinrelationtothecurrent datarate.Large
numbersofsamples,coupledwithslowerdatarates,cantaketimetoacquire.
TheACQUIRE controlstartstheacquisitionprocess.Whenpressed, thesoftwarewillcollectthe
requestednumberofsamplesfromtheADS1298.Allpointscollectedduringanacquisitionprocesswillbe
contiguouspoints.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 9
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
TheCONTINUOUS controlstartsarepeatedacquisitionprocess.Thisfunctionacquirestherequested
samplesandrepeatsthedataacquisitionuntilthebuttonisturnedoff.Withinasingleacquisitioncycle,
thepointswillbecontiguous,butfromacquisitiontoacquisition,theremaybepointsmissing.
TheAnalysisDatainputreferred checkboxchangesthedisplayeddatathat isreadfromtheADC.
Checkingtheboxdisplaysthedatainput referred,whilenotcheckingdisplaysthedataasconverted.
TheShow/PollLeadOffStatus displaysawindow(seeFigure7)that showsthestatusoftheLead-Off
statusregisters, LOFFSTATPandLOFFSTATN,oftheADS1298.Whentheleadfor thechannelis
disconnected,thecorrespondingchannelLEDchangesfromgreentored.
Figure7.Lead-Off StatusRegistersDisplayWindow
3.3 About Tab
TheAbouttabprovidessoftwareinformationtotheuser. Importantsafetywarning,restrictions, and
disclaimersforthesoftwareandhardwareareshownandshouldbefollowedduringtheevaluation of this
product.Additionalindicatorsarepresenttoprovidedeviceinformation(DeviceIDandRev)andsoftware
information(FirmwareVersion).The Abouttabshouldbethefirstscreendisplayedatstartup(see
Figure3).
10 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3.4 ADC Register Tab
TheADCRegistertabprovidescontrolstomanipulateADCcontrolregistersoftheADS1298.Detailsof
thecontrolregistersareprovidedintheproduct datasheet.The ADCRegistertabconsistsofafew
controlsandseveralsub-tabsthatfurther dividethecontrolregistersintodifferent functionsgroups.The
sub-tabsare:
• ChannelRegisterstab(Section3.4.2)
• LOFFandRLDtab(Section3.3)
• GPIOandOtherRegisterstab(Section3.4.4)
• RegisterMaptab(Section3.4.5)
Figure8.Channel RegistersTab
3.4.1 StandbyandResetControls
TheStandbycontrolallowstheusertoplacetheADS1298instandby.
TheResetcontrolallowstheusertoresettheADS1298.The ResetModedetermineswhichmodeis
executedwhentheResetcontrolispressed.DeviceDefaultsresetsthedevicetothedevicedefaults;
ProgrammedDefaultsresetsthedevice, thenwritesthedefaultvaluesfor usingthissoftwareapplication.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 11
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
3.4.2 ChannelRegistersTab(ADCRegister)
TheChannelRegister tabprovidesaccesstocontrolregistersthat controldifferent properties/settingsfor
theADC channels.Thecontrolregisteraregroupedintotwogroups: GlobalChannelRegistersand
ChannelControlRegisters.
3.4.2.1 GlobalChannelRegisters
TheGlobalChannelRegistersboxincludesConfigurationRegister 1(CONFIG1),ConfigurationRegister2
(CONFIG2),ConfigurationRegister3(CONFIG3),andLeadOffControlRegister (LOFF).Theupperhalf
ofFigure8showsthesectionoftheUIpanelthat allowsmanipulationandcontroloftheseregisters.
ConfigurationRegister 1 enablestheusertocontroltheresolutionmode,enablethedaisy-chain
configurationoptions,andprogramthedatarate.
NOTE: SincetheHRbitisnotavailableintheADS1198,theConfigurationRegister1controlwillnot
showthiscontrolwhentestingtheADS1198.
ConfigurationRegister 2 enablestheusertoselect aninternalsquarewavetest sourceamplitude of
±1mVor ±2mVandafrequencyofDC,2Hz(f /221), or4Hz(f /220).
CLK CLK
ConfigurationRegister 3 controlsthebandgapreference(illustratedinFigure9)andrightlegdrive
(RLD)options.Thisregisterenablestheusertoselect betweenanexternalorinternalreferencevoltage,
enable/disabletheinternalreferencebuffer,togglebetweena2.4Vora4.0Voutputvoltage,andto
enable/disabletheRLDaswellaschoosewhethertheRLDvoltageisprovidedinternallyorexternally.
22mF
VCAP1
R1(1)
Bandgap
2.4V or 4V VREFP
R3(1)
10mF
R2(1)
VREFN
AVSS
To ADC Reference Inputs
Figure9.InternalReferenceandBufferConnections
TheLead-OffControlRegister allowstheusertoconfigurethethresholdfor thelead-offcomparator,
resistivepull-uporcurrent-sourceexcitation,thelead-offcurrent magnitude,andDCorACdetection.
Figure10illustratesasimplifieddiagramoftheresistivepull-upandexcitationoptionsfor thelead-off
detectfeature.
12 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
AVDD AVDD
ADS129x ADS129x
10MW
INP INP
PGA PGA
INN INN
10MW
a) Pull-Up/Pull-Down Resistors b) Current Source
Figure10.Lead-OffExcitationOptions
3.4.2.2 ChannelControlRegisters
TheChannelControlRegisters boxallowstheusertouniquelyconfigurethefront-endMUXfor each ADC
channel.Additionally,atthetopoftheChannelControlRegistersbox(seeFigure8)istheoptionto
globallysetallchannelstothesamesetting.Thechannel-specificMUXisillustratedinFigure11.
ADS129x
INT_TEST
MUX
TESTP_PACE_OUT1
INT_TEST
MUX[2:0] = 101
TestP
MUX[2:0] =100
TempP
MvddP(1) MUX[2:0] =011
From LoffP
MUX[2:0] =000
VINP To PgaP
MUX[2:0] =110
MUX[2:0] =010AND
EMI RLD_MEAS MUX[2:0] =001 (AVDD + AVSS)
Filter 2
MUX[2:0] =111
MUX[2:0] =000 MUX[2:0] =001
VINN To PgaN
RLDIN
MUX[2:0] =010AND
From LoffN RLD_MEAS
RLD_REF
MvddN(1) MUX[2:0] =011
MUX[2:0] =100
TempN
MUX[2:0] =101
TestN
INT_TEST
TESTN_PACE_OUT2
INT_TEST
Figure11.InputMultiplexerforaSingleChannel
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 13
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
3.4.3 LOFF andRLDTab(ADCRegister)
TheLOFF andRLDtabprovidescontrolovertheLead-OffDetectionandCurrentControlRegistersand
theRightLegDerivationControlRegisters.ThetabandcontrolsareshowninFigure12.
Figure12.LOFFandRLDTab
3.4.3.1 Lead-OffDetectionandCurrentDirectionControlRegisters
Thefirst twoarraysofcontrols(LeadOffSense)enablelead-offdetectionfor boththepositiveand
negativechannels, LOFF_SENSP andLOFF_SENSN.Bypressingthebuttons,lead-offdetectionis
enabledforeachchannelindividuallyandfor eachinput (positiveandnegative).SetAllLOFFPBits and
SetAll LOFFNBits allowtheusertoturnonoroff alltheenablebitsatonceinsteadofclicking each
individualchannelcontrol.
Thethirdarrayofcontrols(LeadOffCurrentDirection)determinesthecurrent direction usedfor lead-off
detectionwhenanexcitationsignalisselectedasapull-up/pull-downresistor. Eachchanneliscontrolled
individuallybyselectingthebuttonthatcorrespondstothedesiredchanneltomanipulate.Whenthe
buttonisnotilluminated,LOFF_FLIP=0(INPispulled-uptoAVDDandINNispulled-downtoground).
Whenthebuttonispressed/illuminated,LOFF_FLIP=1(INPispulled-downtogroundandINNispulled-
uptoAVDD).Furtherdetailsoftheseregistersandlead-offfunctionarelocatedintheApplications
Sectionofthedevicedatasheet.
14 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
Figure10describesthemodeforLead-Off Detection (thatis,resistiveorcurrentsource) andthe 4-bit
DACsettingstoconfigurethelead-offthreshold.Figure13 illustratestheconnectionsfromthepositive
andnegativeinputstothelead-offcomparators.Theoutputofthecomparatorsisviewedbyusing
Show/PollLeadOffStatus controlasdescribedinSection3.2
LOFF_STATP
VINP
VINN PGA To ADC
LOFF_STATN
4-Bit
DAC COMP_TH[2:0]
Figure13.LOFF_STATPandLOFF_STATNComparators
3.4.3.2 RightLegDriveDerivationControlRegisters
TheRightLegDriveDerivationControlRegistersenabletheusertosetanycombinationofpositiveand/or
negativeelectrodestoderivetheRLDvoltagethat isfedtotheinternalrightlegdriveamplifier.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 15
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
3.4.4 GPIOandOTHERRegistersTab(ADCRegister)
TheGPIOandOther Registers tab,locatedundertheAnalysistab, includescontrolsfor GPIO1 through
GPIO4,respirationphaseandfrequency,routingoftheWilsonamplifiers, andderivationoftheGoldberger
terminals.Figure14showstheGPIOandOTHERRegisters tabandallcontrolscontainedonthe tab.
Figure14.GPIOandOTHERRegisterTab
TheGeneral-PurposeI/ORegister (GPIO) controlsthefourgeneral-purposeI/Opins.EachGPIOcan be
setasaninputoranoutputviaGPIOCxcontrols.If theoutput isselected,the GPIODxcontrolisenabled
allowingtheusertosetthevaluetooutput.If theGPIO isselectedasaninput,the GPIODxcontrolis
disabledandshowsthevalueoftheGPIO. If anyoftheGPIOsareselectedasinputs,the ReadGPIO
controlisenabledwhichallowstheGPIODxvaluestobeupdatedtothecurrent GPIOvalue.
ThePACEDetectRegisterdoesnotenableaspecialPACEmeasurementmode. Theregisterallowsfor
enablingandconfigurationofthePACEamplifiers. PACEAmplifier1canconnecttoinput channels1-4
andPaceAmplifier2canconnecttoinput channels5-8.
TheConfiguration 4Register allowscontrolovertheRespirationFrequency,WCTconnectionto the
RLDandlead-offcomparatorenablestatus.
NOTE: TheRespirationFrequencycontrolisdisablesincethefunctionalityisnotavailableonthe
ADS1298andADS1198.
TheRespirationControlRegister isdisabledfor theADS1298andnotavailablefor theADS1198.
16 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| www.ti.com |                                        |     |     |     |     |     |     | UsingtheADS1298ECG-FESoftware |     |     |
| ---------- | -------------------------------------- | --- | --- | --- | --- | --- | --- | ----------------------------- | --- | --- |
| 3.4.4.1    | WilsonCentralandAugmentedLeadRegisters |     |     |     |     |     |     |                               |     |     |
TheWilsonCentralVoltage(anaveragevoltagebetweentherightarm [RA],leftarm [LA],andleftleg [LL]
connections)canbederivedfromanycombinationofpositiveandnegativeterminalsfromchannels1-4
androutedtotheWCTpin.Likewise,theAugmentedLeads(AVF,AVL,AVR)maybederivedfrom
channels1-4androutedtothenegativeterminalofchannels5,6,and7.Figure15showsthese
configurations;Figure15aillustratesthecentralleadrouting,andFigure15bshowstheaugmented lead
routing.
(a) Wilson Central Lead Routing (b) Wilson Augmented Lead Routing
| IN1P |         |           |         |            | IN1P |         |        |         |         |            |
| ---- | ------- | --------- | ------- | ---------- | ---- | ------- | ------ | ------- | ------- | ---------- |
| IN1N |         |           |         |            | IN1N |         |        |         |         |            |
| IN2P |         |           |         |            | IN2P |         |        |         |         |            |
| IN2N |         |           |         | To Channel | IN2N |         |        |         |         | To Channel |
| IN3P |         |           |         | PGAs       | IN3P |         |        |         |         | PGAs       |
| IN3N |         |           |         |            | IN3N |         |        |         |         |            |
| IN4P |         |           |         |            | IN4P |         |        |         |         |            |
| IN4N |         |           |         |            | IN4N |         |        |         |         |            |
|      | 8:1 MUX | 8:1 MUX   | 8:1 MUX |            |      | 8:1 MUX |        | 8:1 MUX | 8:1 MUX |            |
|      | W       | W         | W       |            |      | W       | W      |         | W       |            |
|      | C Wcta  | C Wctb    | C Wctc  |            |      | C Wcta  | C      | Wctb    | C Wctc  |            |
|      | T       | T         | T       |            |      | T       | T      |         | T       | avF_ch4    |
|      | 1[2:0]  | 2[5:3]    | 2[2:0]  |            |      | 1[2:0]  | 2[5:3] |         | 2[2:0]  |            |
|      |         | 30kW 30kW | 30kW    |            |      |         |        |         |         |            |
| WCT  |         |           |         |            |      |         |        |         |         | ADS1298    |
80pF
|     |     | AVSS |     | ADS1294/6/8 |     | avF_ch6 | avF_ch5 | avF_ch7 |     |     |
| --- | --- | ---- | --- | ----------- | --- | ------- | ------- | ------- | --- | --- |
IN5P
IN5N
|     |     |     |     |     | IN6P |     |     |     |     | To Channel |
| --- | --- | --- | --- | --- | ---- | --- | --- | --- | --- | ---------- |
|     |     |     |     |     | IN6N |     |     |     |     | PGAs       |
IN7P
IN7N
Figure15.WilsonCentralandAugmentedLeadRoutingDiagrams
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 17
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
3.4.5 RegisterMap(ADCRegister)
TheRegisterMaptabisahelpful debugfeaturethat allowstheusertoviewthestateofalltheinternal
registers.ThistabisillustratedinFigure16.RefreshRegisters controlupdatestheregistermapvalues
withthecurrentregistersettingsoftheADS1298.
NOTE: Figure16showsregistersforADS1298.TheRESPregisterisnotpresentfortheADS1198.
Figure16.DeviceRegistersSettings
18 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3.5 Analysis Tab
TheAnalysistabprovidesaccesstothedifferent analysisoptionsthat areavailableusingthesoftware.
Thedifferentanalysesaregroupedbysub-tabs:
• Scopetab(Section3.5.1)
• Histogramtab(Section3.5.2)
• FFTtab(Section3.5.3)
• ECG tab(Section3.5.4)
3.5.1 ScopeTab(Anaysis)
TheScopetoolisusefulforexaminingtheexactamplitudeofthemeasuredinput signalsfromeach
channel.Additionally,userscandeterminethenoisecontributionfromeachchannelatagivenresolution,
andreviewthesamplingrate,thePGAgain, andtheinput signalamplitude. Figure17 illustratesthe
Scopetoolfeatures.
Figure17.ScopeToolFeatures
IntheScopeAnalysiswindow,asFigure18illustrates, thedifferent noiselevelsaredisplayedwhen the
MUX isselectedasInputShort,PGAgainissetto6(default),andthesamplerateissetto500samples
persecond(SPS).
Figure18.ScopeAnalysisTab(NoiseLevelsforEachChannelShown)
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 19
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
3.5.1.1 Zoom Tool
Thezoomtoolallowstheusertozoomineitheronallchannelssimultaneouslyoronasinglechannel.
Figure19showsanexampleofthewaveformexaminationtoolwiththemagnifyingglasszoomedinon
Channel2.Inthiscase,thetoolmakesitmucheasier todeterminethat thenoiseseenontheECG
waveformisaresultof50Hz/60Hzlinecyclenoise.
Figure19.ZoomToolOptions
20 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3.5.2 HistogramTab(Analysis)
TheHistogramtoolisusedprimarilytoseethebinseparationofthedifferent amplitudesoftheECG
waveformharmonics.Figure20illustratesthehistogramoutputfor a12-leadsignal.ThesameECG
SignalZoomanalysismaybeusedonthehistogramplotsfor amore detailedexaminationofthe
amplitudebins.
Figure20.HistogramBinsfor12-LeadECGSignal
Figure21showstheHistogramAnalysiswindowthat isdisplayedwhentheHistogramAnalysisbutton
(atthebottomofthescreeninFigure20)isclicked.Theanalysiswindowshowsthemean, V ,andV
RMS PP
channelamplitudebins.
Figure21.StatisticsfortheSignalAmplitudeofEightECGChannels
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 21
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
3.5.3 FFTTab
TheFFTtoolallowstheusertoexaminethechannel-specificspectrumaswellastypicalfiguresof merit
suchasSNR,THD,ENOB,andCMRR.Eachfeatureisnumberedbelowanddescribedindetailinthe
followingsubsections.Figure22illustratesanFFTplot for anormalelectrodeconfiguration.
Figure22.FFT GraphofNormalElectrodeConfiguration
1-CoherentFrequencyCalculator
CoherentsamplinginanFFT isdefinedasF /F =N /N ,where:
AIN SAMPLE WINDOW TOTAL
• F istheinputfrequency
AIN
• F isthesamplingfrequencyoftheADS1298
SAMPLE
• N isthenumberofoddintegercyclesduringagivensamplingperiod
WINDOW
• N isthenumberofdatapoints(inpowersof2)that isusedtocreatetheFFT
TOTAL
Iftheconditionsforcoherentsamplingcanbemet,theFFTresultsfor aperiodicsignalwillbe
optimized.The Ideal A Frequencyisavaluethat iscalculatedbasedonthesamplingrate, suchthat
IN
thecoherentsamplingcriteriacanbemet.
2-ACAnalysisParameters
Thissectionofthetoolallowstheusertodictatethenumberofharmonics,DCleakagebins,harmonic
leakagebins,andfundamentalleakagebinsthat areusedinthecreationofvarioushistograms.
Pressingthe Windowing button,illustratedinFigure23,allowstheusertoevaluatetheFFTgraph
underavarietyofdifferentwindows.Notethat pressingthe ReferencebuttontogglesbetweendBFS
(decibels,full-scale)anddBc(decibelstocarrier).
Figure23.ACAnalysisParameters:WindowingOptions
22 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3-FFTAnalysis
Pressingthe FFT AnalysisbuttonpullsuptheFFTAnalysiswindowshowninFigure24.Thiswindow
providescalculatedparametersobtainedfromthecollecteddatathat maybeusefulduringevaluation.
Oneofthevaluesincludedinthisanalysisisthechannel-to-channelnoise.
Figure24.FFTAnalysis:InputShortCondition
4-User-DefinedDynamicRange
ThissectionenablestheusertoexaminetheSNRofaspecificchannelwithinagivenfrequencyband
definedbyLowFrequencyandHighFrequency.TheSNRdisplayedinthiswindowshowsunderthe
DynamicRangeheadingasFigure25illustrates.
Figure25.ChangingtheUser-DefinedDynamicRangeforChannel1
5-InputAmplitude
Thisfieldisauserinputthatisimportant for accuratelycalculatingtheCMRRofeachchannel.
6-Zoom Tool
AswiththeAnalysis,Histogram,andScopetool,thiszoomfunctionallowsacloser examinationof the
FFTatfrequenciesofinterest,asshowninFigure26.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 23
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
Figure26.FFTPlot UsingZoomTool
24 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3.5.4 ECGTab(Analysis)
Thistoolallowstheusertoexaminetheinput signalaccordingtothedifferent leadconfigurations. Fora
detaileddescriptionoftheleadconfigurations, seeTable2inSection4.6.Figure27showsLeadsI-IIIand
theAugmentedLeadoutputswiththeinput MUXconfiguredin NormalElectrodemode. Figure27also
showsnumericalannotations1to4,whichhighlight thedifferent featuresofthistool.These featuresare
describedindetailinthefollowingsubsections.
Figure27.ECGDisplayTabShowingLEADI-III andAugmentedLeads
1-PlotSetSelectionFeature
ThePlotSetSelection controlallowstheusertochangethevisualselectionbetween:
• LimbsandAugmentedLeadsdisplaysLEADI, LEADII,LEADIII,aVR,aVL,andAVFsignals,
• ChestLeadsdisplaysV1-V6signals.
NOTE: Fordisplaythatshows6leadscombined,theECGsignalshaveanyDCoffsetremovedand
adifferentoffsetaddedtothesignaltodisplaythesignalsasshown.ToseetherawECG
data,youcanselecttheindividualsignalsasdescribedbelowintheZoomfeature(box4).
2-ECGSeparationFeature
TheECGseparation controltogglestheverticaldistancebetweentheinput plots.Thiscapabilityis
usefulwhenECGsignalsarelargeandrequiremoreseparationtoavoidoverlap,ortocollapsethe
rangebetweensignalswhentheECGsignalsaresmall.
3-PostProcessingFiltersFeature
ThePostProcessingFiltersFeatures controlsprovidesalow-pass, a50Hz/60Hznotch, andahigh-
passdigital filtersforpost-processingthedatafromtheADS1298.Toactivateeachfilter,the Enable
checkboxshouldbechecked.Todisablethefilter,the Enablecheckboxshouldbeunchecked. Any
combinationofthethreedigitalfilterscanbeusedbyenablingtherespectivefilter.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 25
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
Thelow-passfiltercontrolsadigitallow-passfilter,whoseorderandcutofffrequencyarecontrolled
usingthe Filter OrderandCutoffFreq controlsinthelow-passfilterpartofthePost Processing
Filtersgroup(leftsideofthebox).
Thenotchfilterprovidesa50Hz/60Hznotchfilter,whoseorderand50Hz/60Hznotchselection are
controlledusingtheFilter OrderandNotchFreq controlsinthelow-passfilterpartofthe Post
ProcessingFiltersgroup(centerofthebox).
Thehigh-passfiltercontrolsadigitalhigh-passfilter,whoseorderandcutofffrequencyare
controlledusingtheFilter OrderandCutoffFreq controlsinthehigh-passfilterpartofthe Post
ProcessingFiltersgroup(rightsideofthebox).
NOTE: ThedigitalfiltersarenotpartoftheADS1298.ThesearedigitalfiltersimplementedintheUI
toaidintheevaluationoftheADS1298ECG-FE.
4-Zoom Feature
Thezoomfeatureisavailabletoallowtheusertonavigateandviewallsignalsatthesametime,as
showninFigure28.Thistoolallowstheusertozoomin/outonthehorizontalorverticalaxisandpan
leftorrightwhileviewingallECGsignalssimultaneously.
Figure28.ECGSignalZoomFeatureforSixLeads
Additionally,eachECGsignalcanbezoomedindividuallybymovingthemouse(whichappearsasaplus
icon)overtheleadofinterestandclickingonit.AnewwindowopensshowingtherawECGdataasread
fromtheADS1298.Thiswindowprovidescontrolsinthelowerrightcornertozoomin/outorpanright/left
toprovideamoredetailedinspectionof theindividualECGsignal.
Figure29.ECGSignalZoomFeatureforLead1
26 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com UsingtheADS1298ECG-FESoftware
3.6 Save Tab
TheSavetabprovidestheusertheabilitytosavethecollecteddatafor arecordoftheevaluation or
furtheranalysis.Referencetheprevioussectionsfor thelistoftheanlaysisdataavailablefor each
analysis.
TheAnalysistoSavegroupallowstheusertosavethedifferent analysiscalculationsthat wereperformed
onthedata.
• ScopeAnalysis savesthescopeanalysisdataavailablefromthescopeanalysispop-upwindow
• FFTAnalysissavestheFFT analysisdataavailablefromtheFFTanalysispop-upwindow.
• HistogramAnalysissavesthehistogramanalysisdataavailablefromthehistogramanalysispop-up
window.
• Register Settingssavesthecurrent settingsfromtheregistermapandcanbeusefultoobtainthe
registervaluesforyourspecificdeviceconfiguration.Savingtheregistermapinthisformatisnotbe
confusedwithsavingyourregistersettingsfor reloadingintothesoftwareatanothertime(see
Section3.1).
Eachitemwillbesavedifthecorrespondingcheckboxischecked.
TheDatatoSavegroupallowstheusertosavethevariousdatasetscollectedfromtheADS1298.The
CHxcontrolsallowtheusertospecifywhichchannelswillbesavedfor eachofthedataselectionsmade.
• Data-Codesselectstheraw data(incodesformat)tobesavedtoafile.
• Data-Voltsselectstheraw data(convertedtovoltage)tobesavedtoafile.
• FFTDataselectsthecalculatedFFTdatatobesavedtoafile. Note:Thisisnotrawdata;itisthe
frequencybinandmagnitudedatathat wascalculatedbytheUI.
• HistogramDataselectedthecalculatedHistogramdatatobesavedtoafile. Note:Thisisnotraw
data;itisthecodebinandnumberofoccurrencesdatathat wascalculatedbytheUI.
TheUserComments/Notesgroupallowstheusertoindicatea RecordNumber andUser Comments
thataresavedwitheachfile.Thisdatapermitstheusertodistinguishdifferent datasetsfromone
another.
TheDirectorytoSaveFilesisthedirectorywhereallthesavedfileswillbeplaced.Theusercanselecta
directorybypressingthefolderbutton(locatedtotherightofthecontrol). Each datafilethat issavedis
automaticallynamedtopreventoverwritingoffiles.
TheSaveToFile buttonsavesallthedatafilesthat wereselectedusingthecheckboxestotheselected
directory.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 27
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

UsingtheADS1298ECG-FESoftware www.ti.com
Figure30.SaveTab
28 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1x98ECG-FEInputSignals
| 4 ADS1x98ECG-FE |     | Input Signals |
| --------------- | --- | ------------- |
NOTE: BeforeevaluatingspecificECGfunctions,itisrecommendedthattheuseracquiredatawith
inputsshortedinternally.Thisconfigurationensuresthattheboardisoperatingproperly.
| 4.1 Input | Short Testing |     |
| --------- | ------------- | --- |
Bydefault,theEVMpowersupwiththeindividualchannelstoaninternalshort withadatarateof 500SPS
andaPGAgainof6.Oncethe Acquirebutton ispressed, theScopeAnalysisshouldreflectinput-
| referredV    | valueslessthan5µV |       |
| ------------ | ----------------- | ----- |
|              | PP                | PP    |
| 4.2 Internal | Test Signals      | Input |
ConfigurationRegister 2 controlsthesignalamplitude andfrequencyofaninternally-generatedsquare
wavetestsignal.Theprimarypurposeofthistest signalistoverifythefunctionalityofthefront-endMUX,
thePGA,andtheADC.Thetestsignalsmaybeviewedonthe ECGDisplaytab, asFigure31shows.
Detailedinstructionsforusingthe ECGDisplaytabareprovidedinSection3.5.4.
Figure31.ExampleofInternal Test SignalsViewedontheECGDisplayTab
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 29
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1x98ECG-FEInputSignals www.ti.com
4.3 Temperature Sensor
TheinternaltemperaturesensorontheADS1298isshowninFigure32.WhentheinternalMUXisrouted
tothetemperaturesensorinput,theADCinternaltemperatureiscalculatedfromtheADCoutputvoltage
usingEquation1.
AVDD
1x 2x
8x 1x
AVSS
Figure32.InternalTemperatureSensor
Temperature Reading (mV)-145,300mV
Temperature (°C) = + 25°C
490mV/°C
(1)
TheADCcanbeconfiguredtogiveatemperaturereadingbyselectingtheTemperatureSensor option on
theChannelControlRegistersGUI(seeSection3.4.2.2)andverified usingtheScopetabasshownin
Figure33.Thenumber 0.1447V(onthey-axis)canbecalculatedasatemperatureusingEquation 1:
Temperature=(0.1447– 0.145300)/0.00049+25=23.78°C
AmoredetaileddescriptionoftheScopetabisprovidedinSection3.5.1.
Figure33.Eight-ChannelReadofInternalTemperature
30 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1x98ECG-FEInputSignals
4.4 Normal Electrode Input
TheNormal ElectrodeinputontheMUXroutestheinputs(VINPandVINN)differentiallytotheinternal
PGA,asFigure11illustrates.Inthismode, anECG, sinewave,orpulsegeneratormaybeconnected to
testtheADS1298.
Figure34showsatypicalsix-leadoutput whenconnectedtoa5mV ,80BPMECGsignal.
PEAK
Figure34.NormalElectrodeECGConnectioninECGDisplayTab
4.4.1 Capturing12-Lead ECGSignals
Tocapturesignalsfromexternalinputs:
1. ConfiguretheChannelInputcontrolinGloballySetChannelstoNormalElectrode.
2. Connectthe10ECGelectrodesfromtheFlukesimulatortotheEVMthroughtheDB15connector(J1).
RefertoSectionA.5fortheECGcabledetails.TheECGelectrodesignalsarepassedthrough asingle
poleRCfilterfollowedbytheleadconfiguration. ForECGsignalprocessing,theelectrodesignalsare
routedthroughJ5totheADS1298input. Thesignalpathintheboardcanbechosenbyjumper
settings,dependingontheapplication.
4.5 MV Input, RLD Measurement, RLD Positive Electrode Driver, and RLD Negative
DD
Electrode Driver
TheMV inputoptionallowsthemeasurementofthesupplyvoltageV =(AV +AV )/2for channels1,
DD S DD SS
2,5,6,7,and8;however,thesupplyvoltagefor channel3willbeDV /2. Asanexample,inbipolar
DD
supplymode,AV =3.0VandAV =–2.5V.Therefore, withthePGAgain=1,theoutput voltage
DD SS
measuredbytheADCwillbeapproximately0.25V.
TheRLDmeasurementtakesthevoltageattheRLDINpinandmeasuresitonthePGAwithrespectto
(AV +AV )/2.Thisfeatureisbeneficialiftheuserwouldliketooptimize thegainoftheRLDloop.
DD SS
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 31
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1x98ECG-FEInputSignals www.ti.com
Thevoltageusedtoderivetherightlegdrivefor boththepositiveandnegativeelectrodesmayalso be
| measuredwithrespectto(AV |     | +AV )/2. |
| ------------------------ | --- | -------- |
DD SS
| 4.6 Lead | Derivation |     |
| -------- | ---------- | --- |
TheEVMisconfiguredtogeneratethe12ECGsignalsusing10electrodesconnectedtotheeightADC
channels.LeadI,LeadII,andV1-V6arecomputedintheanalogdomain,whiletheaugmentedleadsand
LeadIIIarecomputeddigitally.ThechannelassignmentsaredescribedinTable2.
| • LA=LeftArm  |     |     |
| ------------- | --- | --- |
| • LL=LeftLeg  |     |     |
| • RA=RightArm |     |     |
Table2.ADS1298LeadMeasurements
Lead(1)
ADS1298InputChannels
1 V6=V6–WCT
2 LEADI=LA–RA
3 LEADII=LL–RA
4 V2=V2–WCT
5 V3=V3–WCT
6 V4=V4–WCT
7 V5=V5–WCT
8 V1=V1–WCT
(1) WCT=(LA+RA+LL)/3
Table3.DerivedLeadCalculations
|            | DerivedLead     | FormulaUsedtoCalculate         |
| ---------- | --------------- | ------------------------------ |
|            | LEADIII         | LL-RA-LA=LEADII-LEADI          |
|            | aVR             | RA-(LA+LL)/2=-(LEADI+LEADII)/2 |
|            | aVL             | LA-(RA+LL)/2=LEADI-LEADII/2    |
|            | aVF             | LL-(RA+LA)/2=LEADII-LEADI/2    |
| 4.7 Wilson | Center Terminal | (WCT)                          |
TheWilsonCenterTerminalvoltageis internallygeneratedbytheADS1298device. The WCT1and
WCT2registersprovidecontrolstoselectanyoftheeight inputs(CH1PtoCH4P,CH1MtoCH4M)for
| routingto | thethreeintegratedWCTamplifiers. |     |
| --------- | -------------------------------- | --- |
TheADS1298ECG-FE isconfiguredfor12-leadECGinputs,withthelimbelectrodesconnectedasshown
inTable2.DuringEVMpower-up,thefirmwareconfiguresthedevicetorouteCH2P,CH2M, andCH3P
(RA,LA,LL)totheinternalbuffers.ThisconfigurationgeneratesasignalattheWCTpinequalto(RA +
LA+LL)/3.ByinstallingJP16,theWCTisroutedtothesingle-endedchannelstoachievethedesired
signals.
| 4.8 Right | Leg Drive |     |
| --------- | --------- | --- |
TheRLelectrodeisdrivendirectlybytheRLDsignalgeneratedon-chipbytheADS1298.Thebandwidth
oftheRLDloopisdeterminedbyR8(392kΩ)andC20(10nF).Userscanchangethesevaluestoset the
bandwidthbasedontheirspecificapplication.Theloopstabilityisdeterminedbytheuser’sspecific
system. Therefore,adjustmentofthefeedbackcomponentvaluesmayberequiredtoensurestabilityif
additionalfilteringcomponentsandlongcablesareaddedbeforetheADS1298ECG-FE.
Inatypicalapplication,theRLDsignalisimplementedastheaverageofRA,LA,andLL.Forsystem
flexibility, theADS1298allowstheusertoselectanycombinationoftheelectrodestogeneratethe RLD
(seeADS1298datasheetorADS1198datasheetfor moredetails).
32 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1x98ECG-FEInputSignals
4.8.1 RLDCommonModeVoltage
TheRLDcommonmodevoltagecanbesetto(AVDD+AVSS)/2 ortoanexternallyprovidedsource.Ifthe
applicationrequiresthecommonmodetobesettoanyvoltageotherthanmid-supply, thiscanbe
accomplishedbysettingtheappropriatebitintheConfiguration3Register.OntheADS1298ECG-FE,the
externalRLDREFvoltageissetusingresistor R1andadjustableresistorR2(R1andR2arenotinstalled
bydefault).
Duringpower-up,thefirmwareconfiguresthedevicefor internalRLDREFoperation.ToconfiguretheRLD
circuitrymanually,usethefollowingstepsandthecontrolsfoundonthe ADCRegistertab.
1. VerifythattheChannelInputissettothe NormalElectrodemodefor allchannels(Channel
Registerstab).
2. InCONFIG3controlregister(ChannelRegisters tab):
• EnabletheRLDBuffer(RLDBufferPower =Enable)
• SettheinternalRLDreference(RLDREFSignalSource)
3. Select theelectrodesfortheRLDloopfromtheRightLegDriveDerivationControlRegisters controls
(LOFFandRLD tab)
Oncethesestepsarecompleted,measureandverifythat thevoltageoneithersideofR38isclose to
mid-supply.ThismeasurementconfirmswhethertheRLDloopisfunctional.
Theon-chipRLDsignalcanbefedbackintotheADS1298byshortingJP1.ThisRLDsignalcanthen be
senttothe ADC(tomeasurefordebugpurposes)ortootherelectrodesfor driving(tochangethe
referencedriveincasetheRLelectrodefallsoff).RefertotheADS1298product datasheet orADS1198
datasheetforadditionaldetails.
4.8.2 DrivingtheRLDCableShield
ApartfromtheRLDsignal,theADS1298ECG-FEalsooffersthreeoptionstodrivethecableshield:
• In-phaseRLDsignal
• Out-of-phaseRLDsignal
• BoardAGND
Table4summarizestheconfigurationof JP15 andJP17 for eachoftheoptions.
Table4.RLDJumper Options
ECGCable
ELEC_SHDsignal JP15 JP17
AGND 1-2 Don'tCare
RLD(0:Inphase) 2-3 2-3
RLD(180:Outofphase) 2-3 1-2
4.9 PACE Detection
TheADS1298supportsdataratesupto32kSPSfor softwarePACEdetection,whichtypicallyrequiresa
datarateofatleast8kSPS.
NOTE: TheADS1298ECG-FEdoesnotincludesoftwarePACEdetectionalgorithms.
TheADS1298providestheusertheflexibilityofdoinghardwarePACEdetectionwithexternalcircuitry.
PACE detectioncanbedonesimultaneouslyontwochannels: onefromtheoddchannelsandone from
theevenchannels.RefertotheADS1298product datasheetorADS1198product datasheetfor
additionaldetails.
ToturnonthePACEbufferandselectthechannels, setthe PACERegisterfromtheGPIO andOTHER
Registerstab.ThePGAoutputsoftheselectedchannelsareavailableatconnectorJ5,pins1and2.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 33
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1x98ECG-FEInputSignals www.ti.com
Figure35showsanexamplewaveformcreated byaFlukeMedsim300BprocessedbytheADS1298 at a
datarateof8kSPS.Usinghigherdataratesincreasespower consumptionbecauseallchannelsmust
sampleatthisdataratesimultaneously; thus, thePACEbuffersoffertheflexibilitytoprocessPACE
signalsseparatelyfromtheADS1298.Thesignalmust beACcoupledtoobtainthewaveform\ shown
below.
Figure35.DigitizationofPACESignalUsingADS1298
34 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails
5 ADS1298ECG-FE/ADS1198ECG-FE Hardware Details
TheADS1298ECGfront-endevaluationboardisconfiguredtobeusedwiththeTIMMB0dataconverter
evaluationplatform.TheADS1298ECG-FEboardisafour-layercircuit board.Theboardschematicand
layoutareprovidedinAppendixA.
TheADS1298ECG-FE canbeusedasademonstrationboardfor standard, 12-leadECGapplicationswith
aninputconfigurationof10electrodes.Userscanalsobypassthe12-leadconfigurationandprovide any
typeofsignaldirectlytotheADS1298throughavarietyofhardwarejumpersettings(JP26-33;see
SectionA.4).Externalsupportcircuitsareprovidedfor testingpurposessuchasexternalreferences,
clocks,lead-offresistors,andshielddriveamplifiers.
Figure36showsthefunctionalblockdiagramwithimportantjumpernamesfor theboard.
Figure36.ADS1298ECG-FEFront-EndBlockDiagram
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 35
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails www.ti.com
5.1 Jumper Description
Table5showsthejumpersontheADS1298ECG-FEandoptionsavailablefor eachjumper.
Table5.ADS1x98ECG-FEDefaultJumper/SwitchConfiguration
Jumper Function Settings
JP1 Installed RLDfeedback
1-2:AVDDselectedforbipolarsupplyoperation(AVDD=+2.5V)
JP2 AVDDsupplysource
2-3:AVDDselectedforsinglesupplyoperation(AVDD=+3.0V)
ExternalReferenceConnection Open:Externalreferencenotconnected
JP3
(HeaderNotInstalled)(1) Installed:Externalreferenceconnected
ConnectEVM+5Vrailto Open:EVM+5Vmustbesuppliedexternally
JP4
J4(powerheader) Installed:EVM+5VsuppliedfromJ4(powerheader)
Open:PWDNpincontrolledfromJ5header(pulleduptoDVDD)
JP5 PWDNsource
Installed:Deviceispowereddown(PWDNpin=AGND)
1-2:DC-coupledinputsignals(Pins1-2shortedonPCB)
JP6to InputsignalDC/ACcouples
2-3:AC-coupledinputsignals(RequiresinstallationofheaderandcuttingPCB
JP114 (HeaderNotInstalled)
shortofpin1-2)
1-2:ECGshieldisgrounded(AGND)
JP15 ECGshielddriveconnected 2-3:ECGshieldisconnectedtobuffer(requiredU2installation,otherwiseshield
connectionisopen)
Open:WCTNOTconnectedtoJP26-30andJP33
WilsonCentralTerminal(WCT)
JP16 Installed:WCTconnectedtoJP26-30andJP33forconnectiontoCH1andCH4-8
connection
IN-
ECGshielddrivebuffer 1-2:ECGshielddriveconnectedtoRLDOUT
JP17
input(HeaderNotInstalled)(2) 2-3:ECGshielddriveconnectedtoRLDINV
1-2:CLKconnectedtoDSP(J3.17)
JP18 CLKconnection
2-3:CLKconnectedtoOSC1
1-2:OSC1enabled
JP19 OSC1Enable
2-3:OSC1disabled
1-2:AVSSselectedforsinglesupplyoperation(AVSS=0V(AGND))
JP20 AVSSsupplysource
2-3:AVSSselectedforbipolarsupplyoperation(AVSS=-2.5V)
1-2:CSconnectedtoDSPviaJ3.1
JP21 CSsource
2-3:CSconnectedtoDSPviaJ3.7
1-2:STARTcomesfromJ3.1
JP22 STARTsource
2-3:STARTcomesfromJ3.14
1-2:ExternalMasterClock
JP23 CLKSELsource
2-3:InternalMasterClock(CLKSELcontrolledbyJ3.2(pulleduptoDVDD))
1-2:DVDDsupply=1.8V
JP24 DVDDSupplySelect
2-3:DVDDsupply=3.3V
ExternalReferenceSelection 1-2:SelectU4asreferencesource
JP25
(HeaderNotInstalled)(3) 2-3:SelectedU3asreferencesource
Open:Channelinputnotconnected
CH8-connection
Installed:ChannelinputconnectedtoWCT(requiresJP16tobeinstalled)
JP26
Open:Channelinputnotconnected
CH8+connection
Installed:ChannelinputconnectedtoECG_V1
Open:Channelinputnotconnected
CH7-connection
Installed:ChannelinputconnectedtoWCT(requiresJP16tobeinstalled)
JP27
Open:Channelinputnotconnected
CH7+connection
Installed:ChannelinputconnectedtoECG_V5
Open:Channelinputnotconnected
CH6-connection
Installed:ChannelinputconnectedtoWCT(requiresJP16tobeinstalled)
JP28
Open:Channelinputnotconnected
CH6+connection
Installed:ChannelinputconnectedtoECG_V4
(1) RequiresinstallationofJP25andreferencesU3/U4
(2) RequiresinstallationofU2
(3) RequiresinstallationofJP3andreferencesU3/U4
36 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails
Table5.ADS1x98ECG-FEDefaultJumper/SwitchConfiguration(continued)
Jumper Function Settings
Open:Channelinputnotconnected
CH5-connection
Installed:ChannelinputconnectedtoWCT(requiresJP16tobeinstalled)
JP29
Open:Channelinputnotconnected
CH5+connection
Installed:ChannelinputconnectedtoECG_V3
Open:Channelinputnotconnected
CH4-connection
Installed:ChannelinputconnectedtoWCT(requiresJP16tobeinstalled)
JP30
Open:Channelinputnotconnected
CH4+connection
Installed:ChannelinputconnectedtoECG_V2
Open:Channelinputnotconnected
CH3-connection
Installed:ChannelinputisconnectedtoECG_RA
JP31
Open:Channelinputnotconnected
CH3+connection
Installed:ChannelinputisconnectedtoECG_LL
Open:Channelinputnotconnected
CH2-connection
Installed:ChannelinputisconnectedtoECG_RA
JP32
Open:Channelinputnotconnected
CH2+connection
Installed:ChannelinputisconnectedtoECG_LA
Open:Channelinputnotconnected
CH1-connection
Installed:ChannelinputconnectedtoWCT(requiresJP16tobeinstalled)
JP33
Open:Channelinputnotconnected
CH1+connection
Installed:ChannelinputconnectedtoECG_V6
5.2 Power Supply
TheADS1x98EVMmountsontheMMB0EVMwithconnectorsJ2,J3andJ4.Themainpower supplies
(+5V, +3Vand+1.8V)forthefront-endboardaresuppliedbythehostboard,MMB0,throughconnector
J4.Allotherpowersuppliesneededforthefront-endboardaregeneratedonboardbypower
managementdevices.TheEVMisshippedin+3Vunipolarsupplyconfiguration.
TheADS1298canoperateinasinglesupplywith+3.0Vto+5.0Vanalogsupply(AVDD/AVSS)orbipolar
modesupply(±1.5Vto ±2.5V).Anadditionaldigitalsupplyofand+1.8Vto+3.0Vdigitalsupply(DVDD)is
requiredforoperation.TheADS1298EVM power consumptioncanbemeasuredbyremovingthe JP4
jumperandJP24jumpertoconnectanammeter.ByshortingJP5,theADS1298canbeplacedin
powerdownmodeforlowpowerconsumption.
Test pointsTP5,TP6,TP7,TP8,TP9,TP10,andTP14areprovidedtoverifythepower suppliesvoltages
arecorrect.ThetestpointsandvoltagesareshowninTable6.
Table6. Power-SupplyTestPoints
TestPoint Voltage
TP7 +5.0V
TP9 +1.8V
TP10 +3.3V
TP5 +3.0V
TP13 +2.5V
TP6 –2.5V
TP8 GND
Thefront-endboardmustbeproperlyconfiguredinordertoachievethevariouspower-supplyschemes.
Thedefaultpower-supplysettingfortheADS1298ECG-FEisabipolaranalogsupplyof ±2.5Vand DVDD
ofeither+3Vor+1.8V.Table7 showstheboardandcomponentconfigurationsfor eachanalogpower-
supplyschemeandTable8showstheboardconfigurationsfor thedigitalsupply.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 37
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails www.ti.com
| Table7.AnalogSupplyConfigurations |           |                      |           |     |     | (AVDD/AVSS)         |     |           |
| --------------------------------- | --------- | -------------------- | --------- | --- | --- | ------------------- | --- | --------- |
|                                   |           | UnipolarAnalogSupply |           |     |     | BipolarAnalogSupply |     |           |
| AVDD/AVSS                         |           | 3V                   |           | 5V  |     | ±1.5V               |     | ±2.5V     |
| JP20                              |           | 1-2                  |           | 1-2 |     | 2-3                 |     | 2-3       |
| JP2                               |           | 2-3                  |           | 2-3 |     | 1-2                 |     | 1-2       |
| U7                                | TPS73230  |                      | TPS73250  |     |     | Don'tCare           |     | Don'tCare |
| U9                                | Don'tCare |                      | Don'tCare |     |     | TPS73201            |     | TPS73201  |
| U8                                | Don'tCare |                      | Don'tCare |     |     | TPS72301            |     | TPS72301  |
| R52                               | Don'tCare |                      | Don'tCare |     |     | 21kΩ                |     | 47.5kΩ    |
| R53                               | Don'tCare |                      | Don'tCare |     |     | 78.7kΩ              |     | 43kΩ      |
| R56                               | Don'tCare |                      | Don'tCare |     |     | 23.3kΩ              |     | 49.9kΩ    |
| R57                               | Don'tCare |                      | Don'tCare |     |     | 95.3kΩ              |     | 46.4kΩ    |
C87,C67,C62 NotInstalled NotInstalled NotInstalled NotInstalled
Table8.DigitalSupplyConfigurations(DVDD/DGND)
|     | DVDD |     | +3.0V |     |     | +1.8V |     |     |
| --- | ---- | --- | ----- | --- | --- | ----- | --- | --- |
|     | JP24 |     |       | 1-2 |     | 2-3   |     |     |
5.3 Clock
TheADS1298hasanon-chiposcillator circuit that generatesa2.048MHzclock(nominal).Thisclockcan
varyby±5%overtemperature.Forapplicationsthat requirehigher accuracy,theADS1298also accepts
anexternalclocksignal.TheADS1298ECG-FEprovidesanoptiontotest bothinternalandexternalclock
configurations.Fortheexternalsignal,circuitryisavailabletogeneratetheexternalclockfromanon-
boardoscillatororfromanexternallyconnectedsource.
TheexternaloscillatorincludedontheEVMispoweredfromDVDD, thesamesupplyastheADS1298.
Caremustbetakentoensurethattheexternallysuppliedclockoscillatorcanoperateeitherwith +1.8Vor
+3.0V, dependingontheDVDDsupplyconfiguration.Table9showsthejumpersettingsfor thethree
optionsfortheADS1298clocks.
|              |     | Table9.CLKJumper |     |                  | Options |     |               |     |
| ------------ | --- | ---------------- | --- | ---------------- | ------- | --- | ------------- | --- |
| ADS1298Clock |     | InternalClock    |     | ExternalOSCClock |         |     | ExternalClock |     |
| JP18         |     | NotInstalled     |     |                  | 2-3     |     |               | 1-2 |
1-2(Disable)
| JP19 |     | Don'tCare |     |     |     |     |     | Don'tCare |
| ---- | --- | --------- | --- | --- | --- | --- | --- | --------- |
2-3(Enable)
A2.048MHzoscillatorinstalledontheEVMfor +3VDVDDoperationisFXO-HC735-2.048MHz.If
operationat+1.8VDVDDisdesired,theoscillatorwillneedtobereplaced.SiT8002AC-34-18E-2.048isa
possibleoscillatorfor+1.8VDVDDoperation. TheEVMisshippedwiththeexternaloscillatorenabled.
5.4 Reference
TheADS1298hasanon-chipinternalreferencecircuit that providesreferencevoltagestothedevice.
Alternatively,theinternalreferencecanbepowereddownandVREFPcanbeappliedexternally.This
configurationisachievedwiththeexternalreferencegenerators(U3andU4)anddriverbuffer. NOTE: U3,
U4,anddriverbufferarenotinstalled.Theexternallyprovidedreferencevoltagecanbesettoeither
4.096Vor2.5V,dependingontheanalogsupplyvoltage.MeasureTP3tomakesuretheexternal
referenceiscorrect.ThesettingsfortheexternalreferenceisdescribedinTable10.
38 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails
| Table10.ExternalReferenceJumper |     | Options |     |
| ------------------------------- | --- | ------- | --- |
InternalReference ExternalReference
| ADS1298Reference | VREF=2.5V    | VREFP=4.096V | VREFP=2.5V |
| ---------------- | ------------ | ------------ | ---------- |
| JP25             | Don'tCare    | 2-3          | 1-2        |
| JP3              | NotInstalled | Installed    | Installed  |
ThesoftwareusestheV valuefromthe ReferenceVoltagecontrol(CONFIG3register)in
REF
Section3.4.2.1)tocalculatetheinput-referredvoltagevaluefor allthetests.Thedefaultvalueis2.5V. If
theuserisusinganalternativevalue,thecontrolmust beupdatedtodisplaythecollecteddatato the
proper scale.
5.5 Analog Output Signals
SeveraloutputsignalsfromtheADS1298areprovidedontheJ5header.Table11 liststhevarioustest
signalsandtheirlocationontheheader.ThePACEOUTpinscanalsobeusedasanauxiliarydifferential
inputchannel.Alternativelywithappropriateuserconfiguration,thesepinsmayprovidePACEdetection
forusewithexternalPACEdetectioncircuitry(seePACEDetectRegisterinSection3.4.4).
Table11.TestSignals
| Signal       | J5PinNumber |     | Signal       |
| ------------ | ----------- | --- | ------------ |
| PACEOUT2     | 1           | 2   | PACEOUT1     |
| NOTCONNECTED | 3           | 4   | NOTCONNECTED |
| PWDNB        | 5           | 6   | GPIO4        |
| DAISY_IN     | 7           | 8   | GPIO3        |
| GND          | 9           | 10  | NOTCONNECTED |
5.6 Digital Signals
TheADS1298digital signals(includingSPIinterfacesignals,someGPIO signals,andsomeofthe control
signals)areavailableatconnectorJ3.These signalsareusedtointerfacetotheMMB0boardDSP.The
pinoutfor thisconnectorisgiveninTable12.
Table12.SerialInterfacePinout
| Signal   | J3PinNumber |     | Signal   |
| -------- | ----------- | --- | -------- |
| START/CS | 1           | 2   | CLKSEL   |
| CLK      | 3           | 4   | GND      |
| NC       | 5           | 6   | GPIO1    |
| CS       | 7           | 8   | RESETB   |
| NC       | 9           | 10  | GND      |
| DIN      | 11          | 12  | GPIO2    |
| DOUT     | 13          | 14  | NC/START |
| DRDYB    | 15          | 16  | NC       |
| NC       | 17          | 18  | GND      |
| NC       | 19          | 20  | NC       |
5.7 Analog Input Signals
TheADS1298ECG-FE providesuserstheoptiontofeedinstandardECGsignalsfromapatientsimulator
totheDB15connector(J1),ortofeedinputsfromanyarbitrarysignalsourcedirectlytotheADS1298.
SBAU171D–May2010–RevisedJanuary2016 ADS1298ECG-FE/ADS1198ECG-FE 39
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ADS1298ECG-FE/ADS1198ECG-FEHardwareDetails www.ti.com
5.7.1 PatientSimulatorInput
Theoutputfromanytypicalpatientsimulator canbedirectlyfedintotheDB15connector(J1).Forall
measurementsinthisuserguide,aFlukemedSim300Bsimulator wasusedastheECGsignalsource
(Figure37).ThesimulatoriscapableofgeneratingECGsignalsdownto50µVofamplitude.Particular
attentionmustbegiventothecommon-modevalueoftheinput signalfor properdatacapture. Referto
theADS1298productdatasheetorADS1198product datasheetfor thecommon-moderangeforvarious
programmablegainamplifier(PGA)gainsettings.Section4.4.1explainstheprocessusedtocapture 12-
leadECGdata.
Figure37.FlukeSimulatorConfiguration
5.7.2 ArbitraryInputSignals
ArbitraryinputsignalscanbeconnectedtotheADS1298bybypassingtheDB15connectorandfeeding
thesignaldirectlyatjumpersJP26-JP33. Thisrequirestheremovalofthe16jumpersatJP26-JP33.The
inputsignalmustbeconnecteddifferentiallysinceeachADCchannelinput isdifferential.If itisdesiredto
connectsingle-endedsignals,biasthenegativeinput ofthechannelstoamid-supplyvoltage.
NOTE: Ensurethatthesingle-endedsignalhasanoffsetequaltothevoltagesuppliedatthe
negativeinputofthechannel.
40 ADS1298ECG-FE/ADS1198ECG-FE SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

Appendix A
SBAU171D–May2010–RevisedJanuary2016
Schematics, BOM, Layout, and ECG Cable Details
A.1 Overview
Thissectioncontainsthecompletebillofmaterials,printedcircuit board(PCB)layouts,andschematic
diagramsfortheADS1x98ECG-FE.
NOTE: Boardlayoutsarenottoscale.Theseareintendedtoshowhowtheboardislaidout;they
arenotintendedtobeusedformanufacturingADS1298ECG-FEPCBs.
A.2 ADS1x98ECG-FE Front-End Board Schematics
TheADS1x98ECG-FEschematicisappendedtothisdocument.
SBAU171D–May2010–RevisedJanuary2016 Schematics,BOM,Layout,andECGCableDetails 41
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| BillofMaterials |              |     |     |     |     | www.ti.com |
| --------------- | ------------ | --- | --- | --- | --- | ---------- |
| A.3 Bill        | of Materials |     |     |     |     |            |
Table13liststhebillofmaterialsfortheADS1x98ECG-FE.
|       |        | Table13.BillofMaterials: | ADS1x98ECG-FE |      |            |     |
| ----- | ------ | ------------------------ | ------------- | ---- | ---------- | --- |
| Count | RefDes |                          | Description   | Size | PartNumber | MFR |
| 1     | NA     | Printedwiringboard       |               |      | 6514709    | TI  |
15 C1,C2,C3,C4,C5, Capacitor,ceramic1µF25V10%X5R 0603 GRM188R61E105KA12D Murata
C6,C11,C17,C47,
C48,C49,C52,C58,
C76,C77
| 0   | C7,C8,C15,C19, | Notinstalled |     |     |     |     |
| --- | -------------- | ------------ | --- | --- | --- | --- |
C21,C22,C25,C27,
C34,C36,C38,C40,
C41,C42,C43,C44,
C56,C62,C67,C78,
C79,C81,C83,C85,
C87,C89
1 C9 Capacitor,ceramic22µF6.3V10%X5R 0805 JMK212BJ226KG-T TaiyoYuden
11 C10,C45,C46,C50, Capacitor,ceramic10µF10V10%X5R 0805 GRM219R61A106KE44D Murata
C51,C54,C55,C60,
C61,C65,C66
10 C12,C13,C14,C16, Capacitor,ceramic0.1µF50V10%X7R 0603 GRM188R71H104KA93D Murata
C18,C57,C69,C70,
C94,C95
1 C20 Capacitor,ceramic10nF50V10%X7R 0603 GRM188R71H103KA01D Murata
20 C23,C24,C26,C28, Capacitor,ceramic47pF50V5%C0G 0603 GRM1885C1H470JA01D Murata
C29,C30,C31,C32,
C72,C73,C74,C75,
C80,C82,C84,C86,
C88,C91,C92,C93
| 0   | C33,C35,C37 | Notinstalled |     |     |     |     |
| --- | ----------- | ------------ | --- | --- | --- | --- |
| 0   | C39         | Notinstalled |     |     |     |     |
4 C53,C59,C63,C64 Capacitor,ceramic2.2µF6.3V10%X5R 0603 GRM185R60J225KE26D Murata
2 C68,C71 Capacitor,ceramic100µF10V20%X5R 1210 LMK325BJ107MM-T TaiyoYuden
1 C90 Capacitor,ceramic1000pF50V5%X7R 0603 C1608X7R1H102J TDK
| 0   | D1-D10 | Notinstalled |     |     |     |     |
| --- | ------ | ------------ | --- | --- | --- | --- |
1 J1 Connector,DSUBRcpt15-positionR/APCBSLD D15S13A4GV00LF FCI
2 J2,J3 10x2x0.1female(installedfrombottomside) SSW-110-21-FM-D Samtec
1 J4 5x2x0.1female(installedfrombottomside) SSW-105-21-FM-D Samtec
| 0   | J5  | Notinstalled |     |     |     |     |
| --- | --- | ------------ | --- | --- | --- | --- |
4 JP1,JP4,JP5,JP16 2-positionjumper0.1"spacing TSW-102-07-T-S Samtec
| 0   | JP3              | Notinstalled |     |     |     |     |
| --- | ---------------- | ------------ | --- | --- | --- | --- |
| 0   | JP6,JP7,JP8,JP9, | Notinstalled |     |     |     |     |
JP10,JP11,JP12,
JP13,JP14,JP17,
JP25
9 JP2,JP15,JP18, 3-positionjumper0.1"spacing TSW-103-07-T-S Samtec
JP19,JP20,JP21,
JP22,JP23,JP24
8 JP26,JP27,JP28, 2x2x0.1,2-pindualrowheader TSW-102-07-T-D Samtec
JP29,JP30,JP31,
JP32,JP33
| 5   | L1-L5         | Ferritebead470Ω |     | 0805 | BK2125HM471-T | TaiyoYuden |
| --- | ------------- | --------------- | --- | ---- | ------------- | ---------- |
| 0   | R1,R4,R5,R11, | Notinstalled    |     |      |               |            |
R12,R15,R16,R19,
R20,R23,R24,R27,
R28,R31,R32,R35,
R36,R41,R42,R45-
R51,R54,R55,R58-
R66,R68,R69,R70
| 0   | R2  | Notinstalled |     |     |     |     |
| --- | --- | ------------ | --- | --- | --- | --- |
5 R3,R71,R72,R73, Resistor,0.0Ω1/10W5%SMD 0603 RC0603JR-070RL Yageo
R74
42 Schematics,BOM,Layout,andECGCableDetails SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| www.ti.com |     |     |     |     |     | BillofMaterials |
| ---------- | --- | --- | --- | --- | --- | --------------- |
Table13.BillofMaterials:ADS1x98ECG-FE(continued)
| Count | RefDes |     | Description | Size | PartNumber | MFR |
| ----- | ------ | --- | ----------- | ---- | ---------- | --- |
14 R6,R7,R10,R14, Resistor,10.0kΩ1/10W1%SMD 0603 RC0603FR-0710KL Yageo
R18,R22,R26,R30,
R34,R38,R40,R44,
R67,R75
| 1   | R8  | Resistor,392kΩ1/10W1%SMD |     | 0603 | RC0603FR-07392KL | Yageo |
| --- | --- | ------------------------ | --- | ---- | ---------------- | ----- |
10 R9,R13,R17,R21, Resistor,22.1kΩ1/10W1%SMD 0603 RC0603FR-0722K1L Yageo
R25,R29,R33,R37,
R39,R43
| 1                   | R52 | Resistor,47.5kΩ1/10W1%SMD  |     | 0603 | RC0603FR-0747K5L | Yageo    |
| ------------------- | --- | -------------------------- | --- | ---- | ---------------- | -------- |
| 1                   | R53 | Resistor,43.2kΩ1/10W1%SMD  |     | 0603 | RC0603FR-0743K2L | Yageo    |
| 1                   | R56 | Resistor,49.9kΩ1/10W1%SMD  |     | 0603 | RC0603FR-0749K9L | Yageo    |
| 1                   | R57 | Resistor,46.4kΩ1/10W1%SMD  |     | 0603 | RC0603FR-0746K4L | Yageo    |
| 5 TP1,TP2,TP8,TP11, |     | TestpointPCMini.040"DBLACK |     |      | 5001             | Keystone |
TP12
| 8 TP3,TP4,TP5,TP6, |     | TestpointPCMini.040"DRED |     |     | 5000 | Keystone |
| ------------------ | --- | ------------------------ | --- | --- | ---- | -------- |
TP7,TP9,TP10,
TP13
| 1   | U1(1)    | ICADC24BITSPI32KSPS      |     | 64TQFP  | ADS1298IPAG        | TI        |
| --- | -------- | ------------------------ | --- | ------- | ------------------ | --------- |
|     |          | ICADC16BITSPI8KSPS       |     | 64TQFP  | ADS1198IPAG        | TI        |
| 0   | U2       | NotInstalled             |     |         |                    |           |
| 0   | U3,U4,U5 | Notinstalled             |     |         |                    |           |
| 1   | U6       | ICUNREGCHRGPUMPVINV      |     | SOT23-5 | TPS60403DBVR       | TI        |
| 1   | U7       | ICLDOREG250MA3.0V        |     | SOT23-5 | TPS73230DBVT       | TI        |
| 1   | U8       | ICLDOREGNEG200MAADJ      |     | SOT23-5 | TPS72301DBVT       | TI        |
| 1   | U9       | ICLDOREG250MAADJ-V       |     | SOT23-5 | TPS73201DBV        | TI        |
| 1   | U10      | ICEEPROM256KBIT400KHZ    |     | 8TSSOP  | 24AA256-I/ST       | Microchip |
| 1   | OSC1     | OSC2.0480MHz3.3VHCMOSSMT |     |         | FXO-HC735-2.048MHZ | Fox       |
(1) InstalleddeviceisdeterminedbyInstalledDevicecheckboxlocatednearlowerleftcorneroftheboard.
SBAU171D–May2010–RevisedJanuary2016 Schematics,BOM,Layout,andECGCableDetails 43
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

PrintedCircuitBoardLayout www.ti.com
| A.4 Printed             | Circuit Board | Layout                          |
| ----------------------- | ------------- | ------------------------------- |
| Figure38throughFigure43 |               | showtheADS1x98ECG-FEPCBlayouts. |
Figure39.TopLayer
Figure38.TopComponentPlacement
Figure40.BottomComponentPlacement Figure41.BottomLayer
44 Schematics,BOM,Layout,andECGCableDetails SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com PrintedCircuitBoardLayout
Figure42.InternalGroundPlane(Layer2) Figure43.InternalPowerPlane(Layer3)
SBAU171D–May2010–RevisedJanuary2016 Schematics,BOM,Layout,andECGCableDetails 45
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

ECGCableDetails www.ti.com
A.5 ECG Cable Details
Figure44showsthedetailsoftherecommendedECGcable.
Cabledetails:
• 10-leadECGcableforPhilips/HP-snap,button (PartNo: 010302013);
http://www.biometriccables.com/index.php?productID=692
• 10-leadECGcableforPhilips/HP-Clip-ontype(PartNo: 010303013A);
http://www.biometriccables.com/index.php?productID=693
AnothercompatiblecablefortheADS1298ECG-FE: HP/Philips/Agilent-compatible 10-leadECGcable.
Figure44.ECGCableSchematic
46 Schematics,BOM,Layout,andECGCableDetails SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

Appendix B
SBAU171D–May2010–RevisedJanuary2016
External Optional Hardware
B.1 Optional External Hardware (Not Included)
TheinputoftheADS1x98ECG-FErequiresaDB15connector. Figure45 illustratesthemost optimalcable
connectiontotheADS1298ECG-FE. Figure46andFigure47showtwoalternatewaysthat cablescan be
constructedtointerfacewiththeADS1x98ECG-FE. Figure48 showsanalternatetestingtooltothe
instrumentusedinthetestsforthisuserguide(refertoSection5.7.1).
Figure45.15-Pin,ShieldedConnectorfromBiometricCables
SBAU171D–May2010–RevisedJanuary2016 ExternalOptionalHardware 47
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

OptionalExternalHardware(NotIncluded) www.ti.com
Figure47.15-Pin,TwistedWireCable
Figure46.15-Pin,TwistedWireCabletoBananaJacks
Figure48.CardiosimECGSimulatorTool
48 ExternalOptionalHardware SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

www.ti.com ADS1x98ECG-FEPower-SupplyRecommendations
B.2 ADS1x98ECG-FE Power-Supply Recommendations
IfyouchosetopowertheMMB0boardthroughthewalladapterjack, itmust complywiththefollowing
requirements:
• Outputvoltage:5.5VDCto15VDC
• Maximumoutputcurrent:500mA
• Outputconnector:barrelplug(positivecenter),2.5-mmI.D.x5.5-mmO.D.(9-mminsertiondepth)
• Complieswithapplicableregionalsafetystandards
Figure49showsa+6V power-supplycable(not providedintheEVMkit)connectedtoabatterypackwith
four1.5Vbatteriesconnectedinseries.Connectingtoawall-poweredsourcemakestheADS1x98ECG-
FE moresusceptibleto50Hz/60Hznoisepickup; therefore,for bestperformance,itisrecommendedto
powertheADS1x98ECG-FEwithabatterysource. Thisconfigurationminimizestheamountofnoise
pickupseenatthedigitizedoutputoftheADS1298.
Figure49.RecommendedPower SupplyforADS1x98ECG-FE
SBAU171D–May2010–RevisedJanuary2016 ExternalOptionalHardware 49
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

Appendix C
SBAU171D–May2010–RevisedJanuary2016
Software Installation
C.1 Minimum Requirements
Beforeinstallingthesoftware,verifythatthePCmeetstheminimumrequirementsoutlinedbelow.
• PentiumIII®/Celeron® processor,866MHzorequivalent
• Minimum256MB ofRAM(512MBorgreater recommended)
• USB1.1-compatibleinput
• Harddiskdrivewithatleast200MBfreespace
• Microsoft®Windows® XPoperatingsystemwithSP2orWindows7operatingsystems(WindowsVista
nottested)
• Mouseorotherpointingdevice
• 1280x960minimumdisplayresolution
C.2 Installing the Software
CAUTION
Do not connect the ADS1x98ECG-FE before installing the software on a
suitable PC. Failure to observe this caution may cause Microsoft Windows to
notrecognizetheADS1x98ECG-FE.
ThelatestsoftwareisavailablefromtheADS1x98ECGFE-PDKproductfolderontheTIwebsite.Check
theTI websiteregularlyforupdatedversions.
To installtheADS1298software:
• DownloadthesoftwarefromtheADS1298ECG-FEproductpage
• Clickontheexecutablefile ads129xecg-fe-y.y.y.exe,wherey.y.yrepresentstheversionnumberof
thesoftwareinstaller.
To installtheADS1198software:
• DownloadthesoftwarefromtheADS1198ECG-FEproductpage
• Clickontheexecutablefile ads1198ecg-fe-y.y.y.exe,wherey.y.yrepresentstheversionnumberof
thesoftwareinstaller.
ThenfollowthepromptsillustratedinFigure50throughFigure53.
Youmustacceptthelicenseagreement (showninFigure51)beforeproceedingwiththeinstallation.
50 SoftwareInstallation SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| www.ti.com | InstallingtheSoftware |     |
| ---------- | --------------------- | --- |
Figure50.InitializationofADS1x98ECG-FE Figure51.LicenseAgreement
Figure52.InstallationProcess
Figure53.CompletionofADS1x98ECG-FESoftware
Installation
| SBAU171D–May2010–RevisedJanuary2016 | SoftwareInstallation | 51  |
| ----------------------------------- | -------------------- | --- |
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

InstallingtheSoftware www.ti.com
1 2 3 4 5 6
AVDD AVDD AVDD
AVDD AVSS AVDD AVSS AVDD AVSS
R11 R19 R27
D1 NI D5 NI D8 NI
NI NI NI
D R9 R10 C78 NI R17 R18 C27 NI R25 R26 C79 NI D
22.1k C 47 9 p 1 F 10k C 47 7 p 2 F R N 1 I 2 JP N 1 I E 1 CG_V2 ECG_V2 22.1k C 47 2 p 1 8 F 0k C 47 2 p 6 F R N 2 I 0 JP N 9 I ECG_V4 ECG_V4 22.1k C 47 9 p 1 3 F 0k C 47 3 p 2 F R N 2 I 8 JP N 6 I ECG_V6 ECG_V6
AGND AGND AVSS AGND AGND AVSS AGND AGND AVSS
AVDD
AVDD AVSS AVDD AVDD
AVDD AVSS AVDD AVSS
R15
D2 NI R23 R31
NI D6 NI D9 NI
R 22 1 . 3 1k C 47 8 p 0 F R 10 1 k 4 C 47 7 p 3 F C81 NI R N 1 I 6 J N P I 1 E 0 CG_V3 ECG_V3 R 22 2 . 1 1k C 47 8 p R 1 2 F 0 2 k 2 NI C 47 2 p 9 F C83 NI R N 2 I 4 JP N 8 I ECG_V5 ECG_V5 R 22 2 . 9 1k C 47 8 p R 1 4 F 0 3 k 0 N C 4 I 7 7 p 5 F C85 NI R N 3 I 2 JP N 7 I ECG_V1 ECG_V1
C J1 1 2 3 4 5 6 E E E E E E L L L L L L E E E E E E C C C C C C _ _ _ _ _ _ V V V V V S 2 3 4 5 6 HD AGND AGND AVSS AGND AGND AVSS AGND AGND AVSS C
7
1 1 1 0 2 1 8 9 E E E E L L L L E E E E C C C C _ _ _ _ R L L V A L 1 A
1 1 4 3 ELEC_RL
15
DB15_F-RA
AVDD AVDD
AVDD AVSS AVDD AVSS
R41 R35
D3 NI D7 NI
NI NI
ECG_SHD_DRV JP15 R39 R40 C87 NI R33 R34 C89 NI
B AGND 22.1k C 47 8 p 1 6 F 0k C 47 7 p 4 F R N 4 I 2 JP N 1 I 4 ECG_RA ECG_RA 22.1k C 47 8 p 8 F 10k C 47 3 p 0 F R N 3 I 6 JP N 1 I 2 ECG_LL ECG_LL B
AGND AGND AVSS AGND AGND AVSS
AVDD
AVDD AVSS AVDD AVSS
R45
D4 NI D10
NI NI
R43 R44 C25 NI R37 R38 ECG_RL ECG_RL
22.1k 10k JP13 22.1k 10k
C 47 2 p 4 F C 47 2 p 3 F R N 4 I 6 NI ECG_LA ECG_LA C 47 9 p 2 F C 47 3 p 1 F
ti
A AGND AGND AVSS AGND AGND A
12500TIBoulevard.Dallas,Texas 75243
Title: ADS1298ECGFE
D
En
ra
g
w
in
n
ee
B
r
y
:
:
T
T
o
o
m
m
H
H
e
e
n
n
d
d
r
r
i
i
c
c
k
k
DOCUMENTCONTROL# REV:C
FILE: DATE: 25-Aug-2010 SIZE: SHEET: 1OF:5
1 2 3 4 5 6
52 SoftwareInstallation SBAU171D–May2010–RevisedJanuary2016
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| www.ti.com |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     | InstallingtheSoftware |     |     |
| ---------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --------------------- | --- | --- |
|            | 1   |     |     | 2   |     |     |     | 3   |     |     |     | 4   |     |     |     | 5   |     |                       | 6   |     |
AVDD
C21 NI
AVDD
|     |     |     |     |     |     | 5   | AGND |     |     |     |     |     |     |     |     |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | ---- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
U 2
|     |     |     |     |                          | N I | 1   | R 4 |      |     |     |     |      | R 1 |     |     |     |     |     |     |     |
| --- | --- | --- | --- | ------------------------ | --- | --- | --- | ---- | --- | --- | --- | ---- | --- | --- | --- | --- | --- | --- | --- | --- |
|     |     |     |     | ECG_SHD_DRV ECG_SHD_DRV4 |     | 3   | N I | JP17 |     |     |     |      |     |     |     |     |     |     |     |     |
| D   |     |     |     |                          |     |     |     |      |     |     |     |      | N I |     |     |     |     |     |     | D   |
|     |     |     |     |                          |     | 2   |     | NI   |     |     |     | R5NI |     |     |     |     |     |     |     |     |
C22 NI
JP1
|     |     |     |     |        |        |      | AGND |     |     |      |     |     | R 2 |     |     |     |     |     |     |     |
| --- | --- | --- | --- | ------ | ------ | ---- | ---- | --- | --- | ---- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
|     |     |     |     |        | ECG_RL | AVSS |      |     |     | R3 0 |     |     |     |     |     |     |     |     |     |     |
|     |     |     |     | ECG_RL |        |      |      |     |     |      |     |     | N I |     |     |     |     |     |     |     |
|     |     |     |     |        |        |      |      | C20 | R8  |      |     |     |     | C3  |     |     |     |     |     |     |
392K
|     |     |     | AVDD  |                               |             |     |         | 0.01uF |       |     |              |      | AVSS  | 1 u F      |     |     |     |     |     |     |
| --- | --- | --- | ----- | ----------------------------- | ----------- | --- | ------- | ------ | ----- | --- | ------------ | ---- | ----- | ---------- | --- | --- | --- | --- | --- | --- |
|     |     |     |       |                               |             |     |         |        |       |     | RLDOUT       |      |       | C 1 3 AVSS |     |     |     |     |     |     |
|     |     |     |       |                               |             |     | WCT J P | 16     | AVS S |     | RLDINV RLDIN |      | VCAP3 |            |     |     |     |     |     |     |
|     |     |     | R 5 9 | R 6 0 R 6 1 R 6 2 R 6 3 R 6 4 | R 6 5 R 6 6 |     | JP      | 26     |       |     |              | AVDD |       |            |     |     |     |     |     |     |
|     |     |     | N I   | N I N I N I N I N I           | N I N I     |     |         |        | C 90  |     |              |      |       | 0 . 1 uF   |     |     |     |     |     |     |
|     |     |     |       |                               |             |     | 1 3     | 2 4    | 100pF |     |              |      |       |            |     |     |     |     |     |     |
AVSS
JP27
|     |     |     |     |     |     |     | 1   | 2   |     |     | 61636260211958572023565922555453 |     |     |      | C 1u | 7 F 6 C 1u 7 F 7 |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | -------------------------------- | --- | --- | ---- | ---- | ---------------- | --- | --- | --- | --- |
|     |     |     |     |     |     |     | 3   | 4   |     |     |                                  |     |     | DVDD |      |                  |     |     |     |     |
U 1
C E C G _ V 6 E C G _ V 6 J P 2 8 AD S 1 198 RLDINV U T LD I N EF VD D D D VS S S S S S VS S D D D D VD D A P 3 AVDD1 AVSS1 AGND C
|     |     |       |       |     |             |     | 1   | 2   |     |     | LD O R          | LD R A AV A AV A V A | AV AV A V C |           |           |     |     |     |     |     |
| --- | --- | ----- | ----- | --- | ----------- | --- | --- | --- | --- | --- | --------------- | -------------------- | ----------- | --------- | --------- | --- | --- | --- | --- | --- |
|     |     | E C G | _ V 1 |     | E C G _ V 1 |     | 3   | 4   |     | 6 4 | R               | R                    |             | 5 2 C L   | K S E L   |     |     |     |     |     |
|     |     |       |       |     | E C G _ V 5 |     | J P | 2 9 |     | 1   | W C T           |                      | C L K S E   | L 4 9     | C L K S E | L   |     |     |     |     |
|     |     | E C G | _ V 5 |     |             |     | 1   | 2   |     | 2   | I I N N 8 8 N P |                      | D D G V N D | D D 5 0   |           |     |     |     |     |     |
|     |     | E C G | _ V 4 |     | E C G _ V 4 |     | 3   | 4   |     | 3   | I N 7 N         |                      | A V S       | S 3 2     |           |     |     |     |     |     |
|     |     |       |       |     | E C G _ V 3 |     |     |     |     | 4 5 | I N 7 P         |                      | D G N       | D 4 5 8 1 |           |     |     |     |     |     |
E C G _ V 3 J P 3 0 6 I N 6 N D V D D 4 7 S P I _ D R D Y DV D D
|     |     | E C G | _ V 2 |     | E C G _ V 2           |     | 1   | 2   |     | 7     | I I N N 6 5 P N |     | G / D P R I D O | Y 4 4 6 G P       | I O 4 S G P P I I _ O D 4 | R D Y     |     |     |     |     |
| --- | --- | ----- | ----- | --- | --------------------- | --- | --- | --- | --- | ----- | --------------- | --- | --------------- | ----------------- | ------------------------- | --------- | --- | --- | --- | --- |
|     |     |       |       |     |                       |     | 3   | 4   |     | 8     | I N 5 P         |     | D O U           | T 4 3 S P         | I _ O U T S P I _ O       | U T R 7 5 |     |     |     |     |
|     |     | E C G | _ L L |     | E E C C G G _ _ R L L | A   | J P | 3 1 |     | 1 0 9 | I N 4 N         |     | G P I O         | 2 4 4 5 4 G G P P | I I O O 2 3 G P I O 2     |           |     |     |     |     |
E C G _ R A E C G _ L A 1 1 I N 4 P G P I O 3 4 0 S P I _ C L K G P I O 3 1 0 K
E C G _ L A 1 3 2 4 1 2 I I N N 3 3 N P S C / C L S K 3 9 S P I _ C S S P I _ C L K SPI_CS
|     |     |     |           |     |     |     |     |     |     | 1 3     | I N 2 N                    |                              | C L                    | K 3 7             |                                  |         |      |     |      |     |
| --- | --- | --- | --------- | --- | --- | --- | --- | --- | --- | ------- | -------------------------- | ---------------------------- | ---------------------- | ----------------- | -------------------------------- | ------- | ---- | --- | ---- | --- |
|     |     |     | R N 5 I 8 |     |     |     | J P | 3 2 |     | 1 1 4 5 | I N 2 P UT2 _OUT1          | V3/NC V2/NC                  | S T A R                | T 3 3 4 8 S S P P | I I _ _ S IN T A R T S P I _ S T | A R T D | VDD  |     |      |     |
|     |     |     |           |     |     |     | 1   | 2   |     | 1 6     | I N 1 N O                  |                              | Y_IN T D               | I N 3 3           | S P I _ IN                       |         |      |     |      |     |
|     |     |     |           |     |     |     | 3   | 4   |     | 3 1     | I R N E 1 S P V 1 C E_ C E | A P4 E FP S E FN S D N A P1  | IO 1 IS A P2 S E D G N | D                 |                                  | C 1 1   |      |     |      |     |
|     |     |     |           |     |     |     |     |     |     |         | P A P A                    | V C V R R E V R R E /P W V C | G P D A V C R E        |                   |                                  |         |      |     |      |     |
|     |     |     | AVS S     |     |     |     | J P | 3 3 |     |         |                            |                              | /                      | A G N             | D                                | 1 u F   | OSC1 |     |      |     |
|     |     |     |           |     |     |     | 1   | 2   |     | AGND    |                            |                              |                        |                   |                                  |         | 4    | 1   | JP19 |     |
|     |     |     |           |     |     |     | 3   | 4   |     |         | 18172624292527352842413036 |                              |                        |                   |                                  | AGND    | VDD  | E/D |      |     |
DVDD
| B   |     | AVDD         | AVDD      | AVDD          |     |     |     |       |                 |                 |       |     |       |                        |     |         |                |     |      | B   |
| --- | --- | ------------ | --------- | ------------- | --- | --- | --- | ----- | --------------- | --------------- | ----- | --- | ----- | ---------------------- | --- | ------- | -------------- | --- | ---- | --- |
|     |     |              |           |               |     |     |     |       |                 |                 |       |     |       |                        |     |         | 3 OutputGND    | 2   |      |     |
|     |     | C 6 C 1 6    | C 7 C     | 1 9 C 8 C 1 5 |     |     |     |       |                 |                 |       |     |       | R 6 R 7 CLK            |     |         |                |     |      |     |
|     |     |              |           |               |     |     |     |       | P A C E O U T 2 | P A C E O U T 2 |       |     |       | 10 K 10 K              |     | JP18    | HC735-2.048MHZ |     | AGND |     |
|     |     | 1u F 0. 1 uF | N I N     | I N I N I     |     |     |     |       | P A C E O U T 1 | P A C E O U T 1 |       |     |       |                        |     |         |                |     |      |     |
|     |     |              |           |               |     |     |     |       |                 | V R EFP         |       |     | / R E | S E T                  |     |         |                |     |      |     |
|     |     |              |           |               |     |     |     | VREFP |                 |                 |       |     | G P   | I O 1 / R E S E T      |     | EXT_CLK |                |     |      |     |
|     |     | AVSS         | AGND      | AVSS          |     |     |     |       | C 1             | 0 C 9 5         | VCAP4 |     |       | G P I O 1              |     |         | EXT_CLK        |     |      |     |
|     |     |              |           |               |     |     |     |       | TP3             |                 |       |     | / P W | D N / P W D N          |     |         |                |     |      |     |
|     |     |              |           |               |     |     |     |       | 10 u F          | 0. 1 uF         |       |     | D A   | I S Y _ IN D A I S Y _ | IN  |         |                |     |      |     |
|     |     |              |           |               |     |     |     |       | AVSS            |                 |       |     | VCAP2 |                        |     |         |                |     |      |     |
|     |     | A VDD        | A V DD    | D V DD        |     |     |     |       |                 |                 | C 1   |     |       |                        |     |         |                |     |      |     |
|     |     |              |           |               |     |     |     |       |                 |                 |       | VBG |       | JP5                    |     |         |                |     |      |     |
|     |     | C 4 C14      | C 1 7 C18 | C 1 2 C5      |     |     |     |       |                 |                 | 1u F  |     |       |                        |     |         |                |     |      |     |
AVSS
|     |     | 1uF 0.1uF | 1uF 0.1uF | 0.1uF 1uF |     |     |          |      |     |     |     | C33 | C9   | C2 AGND |     |     |     |                |     |     |
| --- | --- | --------- | --------- | --------- | --- | --- | -------- | ---- | --- | --- | --- | --- | ---- | ------- | --- | --- | --- | -------------- | --- | --- |
|     |     |           |           |           |     | TP1 | TP2 TP11 | TP12 |     |     |     |     |      |         |     |     |     |                |     |     |
|     |     | AVSS      | AGND      | AGND      |     |     |          |      |     |     |     | NI  | 22uF | 1 uF    |     |     |     |                |     |     |
|     |     |           |           |           |     |     |          |      |     |     |     |     |      | AVS S   | J5  |     |     | ECGADCFrontend |     |     |
AVSS
|     |     |     |     |     |     |     | AGND |     |     |     |     |     |     | PACEOUT1 | 2 4 | 1 3 | PACEOUT2 |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | ---- | --- | --- | --- | --- | --- | --- | -------- | --- | --- | -------- | --- | --- | --- |
|     |     |     |     |     |     |     |      |     |     |     |     |     |     | GPIO4    | 6   | 5   | /PWDN    | ti  |     |     |
| A   |     |     |     |     |     |     |      |     |     |     |     |     |     | GPIO3    | 8   | 7   | DAISY_IN |     |     | A   |
10 9
NI
AGND
12500TIBoulevard.Dallas,Texas 75243
Title: ADS1298ECGFE
|                                     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     | En g in ee r | : T o m H e n d r i c k   | DOCUMENTCONTROL#     |       | REV:C        |
| ----------------------------------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | ------------ | ------------------------- | -------------------- | ----- | ------------ |
|                                     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     | D ra w n B   | y : T o m H e n d r i c k |                      |       |              |
|                                     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     | FILE:        |                           | DATE: 25-Aug-2010    | SIZE: | SHEET:2 OF:5 |
|                                     | 1   |     |     | 2   |     |     |     | 3   |     |     |     | 4   |     |     |     | 5            |                           |                      | 6     |              |
| SBAU171D–May2010–RevisedJanuary2016 |     |     |     |     |     |     |     |     |     |     |     |     |     |     |     |              |                           | SoftwareInstallation |       | 53           |
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| InstallingtheSoftware |     |     |     |     |     |     |     |     |     |     |     |     |     |     | www.ti.com |
| --------------------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | ---------- |
|                       | 1   | 2   |     |     |     | 3   |     |     |     | 4   |     | 5   |     | 6   |            |
| D                     |     |     |     |     |     |     |     |     |     |     |     |     |     |     | D          |
ExternalReference
ExternalReferenceDrivers
|     |     |     |     |     | 8 7 U3 |     |     |     |     |     |     |     |     |     |     |
| --- | --- | --- | --- | --- | ------ | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
NI
|     |     |     | AVDD | 1 N/CN/C | N/C    |      |         |       |       | R50 NI    |       |       |     |     |     |
| --- | --- | --- | ---- | -------- | ------ | ---- | ------- | ----- | ----- | --------- | ----- | ----- | --- | --- | --- |
|     |     |     |      | 2 V IN   | OUT 6  |      |         |       |       | C 4 0 NI  |       |       |     |     |     |
|     |     |     |      | 3        |        | C 35 |         |       |       |           |       |       |     |     |     |
|     |     |     | C34  | TE MP    |        | N    | I       |       |       | A V DD    |       |       |     |     |     |
|     |     |     | NI   | 4 GND    | TRIM 5 | C43  |         |       |       |           |       |       |     |     |     |
|     |     |     |      |          |        |      | AVSS    |       |       | C41 NI    |       |       |     |     |     |
|     |     |     |      |          |        | NI   |         |       |       | 7         |       |       |     |     |     |
|     |     |     |      |          |        |      |         |       |       | 8 AGND    |       |       |     |     |     |
|     |     |     | AVSS |          |        |      |         |       |       | 2 U 5     | R 5 1 | J P 3 |     |     |     |
|     |     |     |      |          |        |      |         |       |       | R 4 9 3 6 |       | VREFP |     |     |     |
|     |     |     |      |          | 8 7 U4 |      |         |       |       | N I N I   | N I   | N I   |     |     |     |
| C   |     |     |      |          | NI     |      | J P 25R |       |       |           |       |       |     |     | C   |
|     |     |     |      | N/CN/C   | N/C    |      | 4       | 7     | R 4 8 | 4         |       |       |     |     |     |
|     |     |     | AVDD | 1        |        |      | N I N I |       | N I   |           |       |       |     |     |     |
|     |     |     |      | 2        | 6      |      |         | C 3 8 | C 3 9 | C42 NI    |       |       |     |     |     |
|     |     |     |      | V IN     | OUT    | C 37 |         | N I   | N I   |           |       |       |     |     |     |
|     |     |     |      | 3 TE MP  |        |      |         |       |       | AGND      |       |       |     |     |     |
|     |     |     | C36  | 4        | 5      | N    | I       |       |       |           |       |       |     |     |     |
|     |     |     | NI   | GND      | TRIM   | C44  | AVSS    |       |       |           |       |       |     |     |     |
AVSS
NI
AVSS
NOTINSTALLED
| B   |     |     |     |     |     |     |     |     |     |     |     |     |     |     | B   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
ti
| A   |     |     |     |     |     |     |     |     |     |     |     |     |     |     | A   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
12500TIBoulevard.Dallas,Texas 75243
Title: ADS1298ECGFE
|                         |     |     |     |     |     |     |     |     |     |     |     | En g in ee r : T o m H e n          | d r i c k DOCUMENTCONTROL# |       | REV:C        |
| ----------------------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | ----------------------------------- | -------------------------- | ----- | ------------ |
|                         |     |     |     |     |     |     |     |     |     |     |     | D ra w n B y : T o m H e n          | d r i c k                  |       |              |
|                         |     |     |     |     |     |     |     |     |     |     |     | FILE:                               | DATE: 25-Aug-2010          | SIZE: | SHEET: 3OF:5 |
|                         | 1   | 2   |     |     |     | 3   |     |     |     | 4   |     | 5                                   |                            | 6     |              |
| 54 SoftwareInstallation |     |     |     |     |     |     |     |     |     |     |     | SBAU171D–May2010–RevisedJanuary2016 |                            |       |              |
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

| www.ti.com |     |     |     |     |     |     |     |     |     |     |     | InstallingtheSoftware |     |     |
| ---------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --------------------- | --- | --- |
|            | 1   |     |     | 2   |     |     |     | 3   |     | 4   | 5   |                       | 6   |     |
C48
| D   |     |     |     |     |     |     |     |     |     |     |     |     |     | D   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
1uF
5 3
|     |     | VCC_5v |     |     |     |     | U6  |     | TP4 |     |     |     |     |     |
| --- | --- | ------ | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
CFLY+ CFLY-
|     |     |     | L 1         |      | 2 IN |          | OUT 1 |          | L 2         | VCC_-5v |     |     |     |     |
| --- | --- | --- | ----------- | ---- | ---- | -------- | ----- | -------- | ----------- | ------- | --- | --- | --- | --- |
|     |     |     | C45 3. 3 uH | C46  | C47  |          |       | C49 C50  | 3. 3 uH C51 |         |     |     |     |     |
|     |     |     | 10uF        | 10uF | 1uF  | GND      |       | 1uF 10uF | 10uF        |         |     |     |     |     |
|     |     |     | AGND        |      | AGND | TPS60403 |       | AGND     | AGND        |         |     |     |     |     |
4
AGND
|     |     |     |     |       | U7    |       |            |                 | TP5    |      |     |     |     |     |
| --- | --- | --- | --- | ----- | ----- | ----- | ---------- | --------------- | ------ | ---- | --- | --- | --- | --- |
|     |     |     |     |       |       |       |            | L 3             |        |      |     |     |     |     |
|     |     |     |     |       | 1 IN  | OUT 5 |            |                 | +3.0V  | JP2  |     |     |     |     |
|     |     |     |     | C 5 2 |       |       | C 5        | 3 C 5 4 3. 3 uH | C 5 5  |      |     |     |     |     |
| C   |     |     |     |       |       |       |            |                 |        | AVDD |     |     |     | C   |
|     |     |     |     | 1u F  | 3 E N |       | R 5 4 2. 2 | uF 10 u F       | 10 u F |      |     |     |     |     |
N I
|     |     |     |     | AGND |       |         |     |      | AGND |     |     |     |     |     |
| --- | --- | --- | --- | ---- | ----- | ------- | --- | ---- | ---- | --- | --- | --- | --- | --- |
|     |     |     |     |      | 2 GND | NR/FB 4 |     | AGND |      |     |     |     |     |     |
C57
|     |     |     |     |      | TPS73230 |     | R 5 5 C | 5 6 |     |       |     |     |     |     |
| --- | --- | --- | --- | ---- | -------- | --- | ------- | --- | --- | ----- | --- | --- | --- | --- |
|     |     |     |     | AGND |          |     | N I     |     |     | 0.1uF |     |     |     |     |
N I
AGND
AGND
|     |     |     |     |     | U9  |     |     |     | TP13 |     |     |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | ---- | --- | --- | --- | --- | --- |
L5
|     |     |     |     |     | 1 IN | OUT 5 |       |           | +2.5V |     |     |     |     |     |
| --- | --- | --- | --- | --- | ---- | ----- | ----- | --------- | ----- | --- | --- | --- | --- | --- |
|     |     |     |     | C58 |      |       | C59   | C60 3.3uH | C61   |     |     |     |     |     |
|     |     |     |     | 1uF | 3 EN |       | 2.2uF | 10uF      | 10uF  |     |     |     |     |     |
R56
|     |     |     |     | AGND |          |       | 49.9K |      | AGND |     |     |     |     |     |
| --- | --- | --- | --- | ---- | -------- | ----- | ----- | ---- | ---- | --- | --- | --- | --- | --- |
|     |     |     |     |      | 2        | 4     |       | AGND |      |     |     |     |     |     |
|     |     |     |     |      | GND      | NR/FB |       |      |      |     |     |     |     |     |
| B   |     |     |     |      |          |       | C     | 6 2  |      |     |     |     |     | B   |
|     |     |     |     | AGND | TPS73201 |       | R57   |      |      |     |     |     |     |     |
N I
46.4K
AGND
AGND
TP6
|     |     |     | VCC_-5v |       | U8    |         |       |       |       |      |     |     |     |     |
| --- | --- | --- | ------- | ----- | ----- | ------- | ----- | ----- | ----- | ---- | --- | --- | --- | --- |
|     |     |     |         |       | 2     | 5       |       | L4    | -2.5V |      |     |     |     |     |
|     |     |     |         |       | IN    | OUT     |       | 3.3uH |       |      |     |     |     |     |
|     |     |     |         | C63   |       |         | C64   | C65   | C66   | JP20 |     |     |     |     |
|     |     |     |         | 2.2uF | 3     |         | 2.2uF | 10uF  | 10uF  | AVSS |     |     |     |     |
|     |     |     |         |       | EN    |         | R52   |       |       |      |     |     |     |     |
|     |     |     |         | AGND  |       |         | 47.5K |       | AGND  |      |     |     |     |     |
|     |     |     |         |       |       |         |       | AGND  |       | AGND |     |     |     |     |
|     |     |     |         |       | 1 GND | NR/FB 4 |       |       |       |      |     |     |     |     |
ECGPowerSupplies
|     |     |     |     |      | TPS72301 |     |       | C 6 7 |     |     |     |     |     |     |
| --- | --- | --- | --- | ---- | -------- | --- | ----- | ----- | --- | --- | --- | --- | --- | --- |
|     |     |     |     | AGND |          |     | R53   | N I   |     |     |     |     |     |     |
|     |     |     |     |      |          |     | 43.2K |       |     |     |     | ti  |     |     |
AGND
| A   |     |     |     |     |     |     |     |     |     |     |     |     |     | A   |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
AGND
12500TIBoulevard.Dallas,Texas 75243
Title: ADS1298ECGFE
|                                     |     |     |     |     |     |     |     |     |     |     | En g in ee r : T o m H e n | d r i c k DOCUMENTCONTROL# |       | REV:C        |
| ----------------------------------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | -------------------------- | -------------------------- | ----- | ------------ |
|                                     |     |     |     |     |     |     |     |     |     |     | D ra w n B y : T o m H e n | d r i c k                  |       |              |
|                                     |     |     |     |     |     |     |     |     |     |     | FILE:                      | DATE: 25-Aug-2010          | SIZE: | SHEET: 4OF:5 |
|                                     | 1   |     |     | 2   |     |     |     | 3   |     | 4   | 5                          |                            | 6     |              |
| SBAU171D–May2010–RevisedJanuary2016 |     |     |     |     |     |     |     |     |     |     |                            | SoftwareInstallation       |       | 55           |
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

InstallingtheSoftware www.ti.com
| 1   |     | 2   |     |     | 3   |     |     |     | 4   |     |     |     | 5   |     | 6   |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
D D
MDKInterfaceConnectors
DVDD
DVDD
|     |     |     |     |     |     |     |     |     |     |     |     | C71 | 100uF |     |     |     |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | ----- | --- | --- | --- |
R67
|     |        |      |     |      |     |        | 10K |          |       |     |            | C70  |            |     |     |     |
| --- | ------ | ---- | --- | ---- | --- | ------ | --- | -------- | ----- | --- | ---------- | ---- | ---------- | --- | --- | --- |
|     |        |      |     |      |     |        |     |          |       | J4  |            |      | 0.1uF AGND |     |     |     |
|     |        |      |     |      |     | JP 2 3 | CLK | S E L    |       | 2 1 | TP 9       |      |            |     |     |     |
|     | 1 J2 2 |      |     | 1 J3 | 2   |        |     |          | T P 7 | 4 3 |            |      |            |     |     |     |
|     | 3 4    | JP21 |     | 3    | 4   |        |     | VCC_ 5 v |       | 6 5 | V C C_1.8V | JP24 |            |     |     |     |
5 6 SPI_CS S P I _ C L K 5 6 G P IO 1 JP4 8 10 7 9 VCC_3.3V TP10
|     | 7 8           |     |            | 7     | 8       | /R E S ET |     |       |       |     |       |     |     |     |     |     |
| --- | ------------- | --- | ---------- | ----- | ------- | --------- | --- | ----- | ----- | --- | ----- | --- | --- | --- | --- | --- |
|     | 1 1 9 1 1 0 2 |     |            | 1 1 9 | 1 1 0 2 |           |     | C 6 8 | C 6 9 |     | T P 8 |     |     |     |     |     |
|     | 1 3 1 4       |     | S P I _ IN | 1 3   | 1 4     | G P I O 2 |     |       |       |     |       |     |     |     |     |     |
C 1 5 1 6 S S P P I I _ _ D O R U D T Y 1 5 1 6 V C C_3.3V 1 0 0 uF 0. 1 u F C
|     | 1 7 1 8 |     | E X T _ C | L K 1 7 | 1 8 |     |     |          |     |          |     |     |     |     |     |     |
| --- | ------- | --- | --------- | ------- | --- | --- | --- | -------- | --- | -------- | --- | --- | --- | --- | --- | --- |
|     | 1 9 2 0 |     |           | 1 9     | 2 0 |     | C94 | 0 .1 u F |     |          |     |     |     |     |     |     |
|     |         |     |           |         |     |     | U10 |          |     | VCC_3.3V |     |     |     |     |     |     |
DummyConnector
|     |     | SPI_START JP22 |     |     |      | SCL | 6 8 V C C      | A 0 1 2 | R R 6 6 8 9 | N I     |     |     |     |     |     |     |
| --- | --- | -------------- | --- | --- | ---- | --- | -------------- | ------- | ----------- | ------- | --- | --- | --- | --- | --- | --- |
|     |     |                |     |     | AGND | SDA | 5 S C L        | A 1 3   | R 7 0       | N N I I |     |     |     |     |     |     |
|     |     |                |     |     |      |     | 7 S W D P A GN | A D 2 4 |             |         |     |     | R74 |     |     |     |
|     |     |                |     |     |      |     | 24AA256-I/ST   |         |             |         |     |     | 0   |     |     |     |
R71R72R73 0 0 0
AGND
NOTE:J2,J3,J4femaleconnectorsshouldbepopulatedfromthebottomside!
B B
ECGMDKBoardInterfaceAdapter
ti
A A
12500TIBoulevard.Dallas,Texas 75243
Title: ADS1298ECGFE
|                         |     |     |     |     |     |     |     |     |     |     |     |     | En g in ee r : T o m H e n d        | r i c k DOCUMENTCONTROL# |       | REV:C        |
| ----------------------- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | ----------------------------------- | ------------------------ | ----- | ------------ |
|                         |     |     |     |     |     |     |     |     |     |     |     |     | D ra w n B y : T o m H e n d        | r i c k                  |       |              |
|                         |     |     |     |     |     |     |     |     |     |     |     |     | FILE:                               | DATE: 25-Aug-2010        | SIZE: | SHEET:5 OF:5 |
| 1                       |     | 2   |     |     | 3   |     |     |     | 4   |     |     |     | 5                                   |                          | 6     |              |
| 56 SoftwareInstallation |     |     |     |     |     |     |     |     |     |     |     |     | SBAU171D–May2010–RevisedJanuary2016 |                          |       |              |
SubmitDocumentationFeedback
Copyright©2010–2016,TexasInstrumentsIncorporated

IMPORTANT NOTICE AND DISCLAIMER
TI PROVIDES TECHNICAL AND RELIABILITY DATA (INCLUDING DATASHEETS), DESIGN RESOURCES (INCLUDING REFERENCE
DESIGNS), APPLICATION OR OTHER DESIGN ADVICE, WEB TOOLS, SAFETY INFORMATION, AND OTHER RESOURCES “AS IS”
AND WITH ALL FAULTS, AND DISCLAIMS ALL WARRANTIES, EXPRESS AND IMPLIED, INCLUDING WITHOUT LIMITATION ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT OF THIRD
PARTY INTELLECTUAL PROPERTY RIGHTS.
These resources are intended for skilled developers designing with TI products. You are solely responsible for (1) selecting the appropriate
TI products for your application, (2) designing, validating and testing your application, and (3) ensuring your application meets applicable
standards, and any other safety, security, regulatory or other requirements.
These resources are subject to change without notice. TI grants you permission to use these resources only for development of an
application that uses the TI products described in the resource. Other reproduction and display of these resources is prohibited. No license
is granted to any other TI intellectual property right or to any third party intellectual property right. TI disclaims responsibility for, and you fully
indemnify TI and its representatives against any claims, damages, costs, losses, and liabilities arising out of your use of these resources.
TI’s products are provided subject to TI’s Terms of Sale, TI’s General Quality Guidelines, or other applicable terms available either on
ti.com or provided in conjunction with such TI products. TI’s provision of these resources does not expand or otherwise alter TI’s applicable
warranties or warranty disclaimers for TI products. Unless TI explicitly designates a product as custom or customer-specified, TI products
are standard, catalog, general purpose devices.
TI objects to and rejects any additional or different terms you may propose.
IMPORTANT NOTICE
Copyright © 2026, Texas Instruments Incorporated
Last updated 10/2025