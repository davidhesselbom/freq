#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "ui_mainwindow.h"
#include "sawe/configuration.h"
#include "ui/mainwindow.h"

// gpumisc
#ifdef USE_CUDA
#include <CudaProperties.h>
#endif
#include <cpuproperties.h>

// license
#include "sawe/reader.h"

namespace Tools
{

AboutDialog::AboutDialog(Sawe::Project* project) :
    QDialog(project->mainWindow()),
    ui(new Ui::AboutDialog)
{
    setWindowModality( Qt::WindowModal );

    ui->setupUi(this);

#ifdef _MSC_VER
    ui->textEdit->setHtml( ui->textEdit->toHtml().replace("file:///usr/share/sonicawe/license/license.txt", "file:///license.txt"));
#endif

    QPalette p = ui->textEdit->palette();
    p.setColor( QPalette::Base, p.color(QPalette::Window) );
    ui->textEdit->setPalette( p );

    Ui::MainWindow* main_ui = project->mainWindow()->getItems();
    connect(main_ui->actionAbout, SIGNAL(triggered()), SLOT(show()));
    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(hide()));
    connect(ui->buttonBox, SIGNAL(rejected()), SLOT(hide()));

    showEvent(0);
}


void AboutDialog::
        showEvent(QShowEvent *)
{
    ui->labelVersion->setText( QString::fromStdString( Sawe::Configuration::version_string() ) );
    ui->labelTimestamp->setText( QString("Built on %1 at %2 from revision %3.")
                                 .arg(Sawe::Configuration::build_date().c_str())
                                 .arg(Sawe::Configuration::build_time().c_str())
                                 .arg(Sawe::Configuration::revision().c_str()) );
    ui->labelLicense->setText( Sawe::Reader::reader_text().c_str() );
    if (Sawe::Reader::reader_title() == Sawe::Reader::reader_text() )
        ui->labelLicense->clear();

    int cores = Sawe::Configuration::cpuCores();

#ifdef USE_CUDA
    size_t free=0, total=0;
    cudaMemGetInfo(&free, &total);
    cudaDeviceProp prop = CudaProperties::getCudaDeviceProp();

    ui->labelSystem->setText(QString(
            "Using CPU with %13 core%14.\n"
            "Using GPU (%1 of %2) %3.\n"
            "%4 free of %5 total graphics memory.\n"
            "Gpu Gflops: %6\n"
            "Gpu memory speed: %7/s (estimated)\n"
            "Cpu memory speed: %8/s (estimated)\n"
            "Cuda compute capability: %9.%10\n"
            "Cuda driver version: %11\n"
            "Cuda runtime version: %12\n")
                             .arg(1+CudaProperties::getCudaCurrentDevice())
                             .arg(CudaProperties::getCudaDeviceCount())
                             .arg(prop.name)
                             .arg(DataStorageVoid::getMemorySizeText( free, 4 ).c_str())
                             .arg(DataStorageVoid::getMemorySizeText( total, 4 ).c_str())
                             .arg(CudaProperties::flops(prop)*1e-9, 0, 'f', 0)
                             .arg(DataStorageVoid::getMemorySizeText( CudaProperties::gpu_memory_speed(), 1, 'f' ).c_str())
                             .arg(DataStorageVoid::getMemorySizeText( CpuProperties::cpu_memory_speed(), 1, 'f' ).c_str())
                             .arg(prop.major).arg(prop.minor)
                             .arg(CudaProperties::getCudaDriverVersion())
                             .arg(CudaProperties::getCudaRuntimeVersion())
                             .arg(cores).arg(cores==1?"":"s")
                             );
#else
    ui->labelSystem->setText(QString(
            "Using CPU with %2 core%3.\n"
            "Cpu memory speed: %1 GB/s (estimated)\n")
                             .arg(CpuProperties::cpu_memory_speed()*1e-9, 0, 'f', 1)
                             .arg(cores).arg(cores==1?"":"s")
                             );
#endif
}


AboutDialog::~AboutDialog()
{
    delete ui;
}

} // namespace Tools
