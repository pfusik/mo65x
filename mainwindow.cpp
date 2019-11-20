#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  initConfigStorage();
  startEmulator();

  cpuWidget = new CpuWidget(this, emulator->memoryView());
  this->addDockWidget(Qt::RightDockWidgetArea, cpuWidget);

  assemblerWidget = new AssemblerWidget;
  memoryWidget = new MemoryWidget(this, emulator->memoryView());
  viewWidget = new CentralWidget(this, assemblerWidget, memoryWidget);
  setCentralWidget(viewWidget);

  pollTimer = new QTimer(this);
  // pollTimer->start(1000);

  connect(cpuWidget, &CpuWidget::startStepExecution, [&] { emulator->startExecution(false); });
  connect(cpuWidget, &CpuWidget::startContinuousExecution, [&] { emulator->startExecution(true); });
  connect(cpuWidget, &CpuWidget::programCounterChanged, emulator, &Emulator::changeProgramCounter);
  connect(cpuWidget, &CpuWidget::stackPointerChanged, emulator, &Emulator::changeStackPointer);
  connect(cpuWidget, &CpuWidget::registerAChanged, emulator, &Emulator::changeAccumulator);
  connect(cpuWidget, &CpuWidget::registerXChanged, emulator, &Emulator::changeRegisterX);
  connect(cpuWidget, &CpuWidget::registerYChanged, emulator, &Emulator::changeRegisterY);
  connect(cpuWidget, &CpuWidget::clearStatisticsRequested, emulator, &Emulator::clearStatistics);

  connect(cpuWidget, &CpuWidget::stopExecutionRequested, emulator, &Emulator::stopExecution, Qt::DirectConnection);
  connect(cpuWidget, &CpuWidget::resetRequested, emulator, &Emulator::triggerReset, Qt::DirectConnection);
  connect(cpuWidget, &CpuWidget::nmiRequested, emulator, &Emulator::triggerNmi, Qt::DirectConnection);
  connect(cpuWidget, &CpuWidget::irqRequested, emulator, &Emulator::triggerIrq, Qt::DirectConnection);

  connect(emulator, &Emulator::stateChanged, cpuWidget, &CpuWidget::updateState);
  connect(emulator, &Emulator::memoryContentChanged, cpuWidget, &CpuWidget::updateMemoryView);
  connect(emulator, &Emulator::memoryContentChanged, memoryWidget, &MemoryWidget::updateMemoryView);
  connect(emulator, &Emulator::operationCompleted, this, &MainWindow::showResult);

  connect(assemblerWidget, &AssemblerWidget::fileLoaded, this, &MainWindow::changeAsmFileName);
  connect(assemblerWidget, &AssemblerWidget::fileSaved, this, &MainWindow::changeAsmFileName);
  connect(assemblerWidget, &AssemblerWidget::operationCompleted, this, &MainWindow::showResult);
  connect(assemblerWidget, &AssemblerWidget::machineCodeGenerated, emulator, &Emulator::loadMemory);
  connect(assemblerWidget, &AssemblerWidget::machineCodeGenerated, emulator, &Emulator::changeProgramCounter);
  connect(assemblerWidget, &AssemblerWidget::programCounterChanged, emulator, &Emulator::changeProgramCounter);

  connect(memoryWidget, &MemoryWidget::loadFromFileRequested, emulator, &Emulator::loadMemoryFromFile);
  connect(memoryWidget, &MemoryWidget::saveToFileRequested, emulator, &Emulator::saveMemoryToFile);

  emulator->stateChanged(emulator->currentState());

  if (!config.asmFileName.isEmpty()) assemblerWidget->loadFile(config.asmFileName);
}

MainWindow::~MainWindow() {
  emulatorThread.quit();
  emulatorThread.wait();
  delete ui;
}

void MainWindow::changeAsmFileName(const QString& fileName) {
  config.asmFileName = fileName;
  configStorage->write(config);

  QFileInfo fileInfo(fileName);
  setWindowTitle("mo65plus: " + fileInfo.fileName());
}

void MainWindow::showResult(const QString& message, bool success) {
  if (success) {
    ui->statusbar->setStyleSheet("color: darkseagreen");
    ui->statusbar->showMessage(message, 10000);
  } else {
    ui->statusbar->setStyleSheet("color: red");
    ui->statusbar->showMessage(message);
  }
}

void MainWindow::initConfigStorage() {
  auto appDir = QDir(QDir::homePath() + "/.mo65plus");
  if (!appDir.exists()) appDir.mkpath(".");

  configStorage = new FileDataStorage<Config>(appDir.filePath("config.json"));
  config = configStorage->readOrCreate();
}

void MainWindow::startEmulator() {
  emulator = new Emulator();
  emulator->moveToThread(&emulatorThread);
  connect(&emulatorThread, &QThread::finished, emulator, &Emulator::deleteLater);
  emulatorThread.start();
}
