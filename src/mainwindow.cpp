#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    QGLFormat format;
    format.setVersion(4,1);
    format.setProfile(QGLFormat::CoreProfile);

    m_openGLWidget = new OpenGLWidget(format,this);
    ui->gridLayout->addWidget(m_openGLWidget,0,1,2,2);


    // A toolbar used to hold the button associated with different elements in the scene e.g. lighting, mesh options
    m_toolBar = new QToolBar();
    m_toolBar->setOrientation(Qt::Vertical);
    ui->gridLayout->addWidget(m_toolBar, 0, 0, 2, 1);

    //--------------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------Light functionality------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------
    QPixmap light("icons/light.png");
    QIcon lightBtnIcon(light);
    m_lightToolbarButton = new QToolButton();
    m_lightToolbarButton->setIcon(lightBtnIcon);
    m_lightToolbarButton->setToolTip("Light options");
    m_toolBar->addWidget(m_lightToolbarButton);
    m_toolBar->addSeparator();

    // Create the dock for the light options
    m_lightWidget = new QWidget();
    m_lightDockGridLayout = new QGridLayout();
    m_lightWidget->setLayout(m_lightDockGridLayout);
    m_lightDockWidget = new QDockWidget("Light Attributes");

    // Creae a drop down menu for the ligth type
    m_lightTypeLabel = new QLabel("Type:");
    m_lightTypeComboBox = new QComboBox();

    m_lightTypeComboBox->setMinimumWidth(150);
    m_lightTypeComboBox->addItem("Ambient Light");
    m_lightTypeComboBox->addItem("Area Light");
    m_lightTypeComboBox->addItem("Directional Light");
    m_lightTypeComboBox->addItem("Point Light");
    m_lightTypeComboBox->addItem("Spot Light");

    m_lightColourDialog = new QColorDialog();
    m_lightColourDialog->setHidden(true);
    m_lightColourButton = new QPushButton("Colour");
    m_lightIntensityLabel = new QLabel("Intensity:");
    m_lightIntensitySlider = new QSlider();
    m_lightIntensitySlider->setOrientation(Qt::Horizontal);

    // Add widgets to the light dock
    m_lightDockGridLayout->addWidget(m_lightTypeLabel, 0, 0, 1, 1);
    m_lightDockGridLayout->addWidget(m_lightTypeComboBox, 0, 1, 1, 1);
    m_lightDockGridLayout->addWidget(m_lightColourButton, 1, 0, 1, 2);
    m_lightDockGridLayout->addWidget(m_lightColourDialog);
    m_lightDockGridLayout->addWidget(m_lightIntensityLabel, 2, 0, 1, 1);
    m_lightDockGridLayout->addWidget(m_lightIntensitySlider, 2, 1, 1, 1);

    // Add a spacer on the bottom
    m_lightSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_lightDockGridLayout->addItem(m_lightSpacer, 3, 0, 1, 1);
    m_lightDockWidget->setWidget(m_lightWidget);
    m_lightDockWidget->setHidden(true);
    this->addDockWidget(Qt::RightDockWidgetArea, m_lightDockWidget);

    //--------------------------------------------------------------------------------------------------------------------
    // ------------------------------------------------Mesh functionality-------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------
    // Button to access the mesh specific parameters
    QPixmap mesh("icons/teapot.png");
    QIcon meshBtnIcon(mesh);
    m_meshToolbarButton = new QToolButton();
    m_meshToolbarButton->setIcon(meshBtnIcon);
    m_meshToolbarButton->setToolTip("Mesh options");
    m_toolBar->addWidget(m_meshToolbarButton);
    m_toolBar->addSeparator();

    m_meshWidget = new QWidget();
    m_meshDockGridLayout = new QGridLayout();
    m_meshWidget->setLayout(m_meshDockGridLayout);
    m_meshDockWidget = new QDockWidget("Mesh Attributes");

    m_meshTranslateLabel = new QLabel("Translate");
    m_meshDockGridLayout->addWidget(m_meshTranslateLabel, 0, 0, 1, 1);
    m_meshTranslateXDSpinBox = new QDoubleSpinBox();
    m_meshDockGridLayout->addWidget(m_meshTranslateXDSpinBox, 0, 1, 1, 1);
    m_meshTranslateYDSpinBox = new QDoubleSpinBox();
    m_meshDockGridLayout->addWidget(m_meshTranslateYDSpinBox, 0, 2, 1, 1);
    m_meshTranslateZDSpinBox = new QDoubleSpinBox();
    m_meshDockGridLayout->addWidget(m_meshTranslateZDSpinBox, 0, 3, 1, 1);
    m_meshRotateLabel = new QLabel("Rotate");
    m_meshDockGridLayout->addWidget(m_meshRotateLabel, 1, 0, 1, 1);
    m_meshRotateXDSpinBox = new QDoubleSpinBox();
    m_meshDockGridLayout->addWidget(m_meshRotateXDSpinBox, 1, 1, 1, 1);
    m_meshRotateYDSpinBox = new QDoubleSpinBox();
    m_meshDockGridLayout->addWidget(m_meshRotateYDSpinBox, 1, 2, 1, 1);
    m_meshRotateZDSpinBox = new QDoubleSpinBox();
    m_meshDockGridLayout->addWidget(m_meshRotateZDSpinBox, 1, 3, 1, 1);

    m_meshSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_meshDockGridLayout->addItem(m_meshSpacer, 5, 0, 2, 1);
    m_meshDockWidget->setWidget(m_meshWidget);
    m_meshDockWidget->setHidden(true);
    this->addDockWidget(Qt::RightDockWidgetArea, m_meshDockWidget);

    //--------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------Connections-------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------

    connect(m_lightToolbarButton, SIGNAL(clicked(bool)), m_lightToolbarButton, SLOT(setChecked(bool)));
    connect(m_lightToolbarButton, SIGNAL(clicked()), m_lightDockWidget, SLOT(show()));
    connect(m_lightColourButton, SIGNAL(clicked()), m_lightColourDialog, SLOT(show()));
    connect(m_meshToolbarButton, SIGNAL(clicked(bool)), m_meshToolbarButton,  SLOT(setChecked(bool)));
    connect(m_meshToolbarButton, SIGNAL(clicked()), m_meshDockWidget, SLOT(show()));
}

MainWindow::~MainWindow(){
    delete m_meshToolbarButton;
    delete m_meshTranslateXDSpinBox;
    delete m_meshTranslateYDSpinBox;
    delete m_meshTranslateZDSpinBox;
    delete m_meshRotateXDSpinBox;
    delete m_meshRotateYDSpinBox;
    delete m_meshRotateZDSpinBox;
    delete m_meshRotateLabel;
    delete m_meshTranslateLabel;
    delete m_meshDockGridLayout;
    delete m_meshWidget;
    delete m_meshDockWidget;
//    delete m_lightSpacer;
    delete m_lightIntensityLabel;
    delete m_lightIntensitySlider;
    delete m_lightColourButton;
    delete m_lightColourDialog;
    delete m_lightTypeComboBox;
    delete m_lightTypeLabel;
    delete m_lightDockGridLayout;
    delete m_lightWidget;
    delete m_lightDockWidget;
    delete m_lightToolbarButton;
    delete m_toolBar;
    delete ui;
    delete m_openGLWidget;
}
