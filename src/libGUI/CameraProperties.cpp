#include "CameraProperties.h"

#include "ui_CameraProperties.h"

CameraProperties::CameraProperties(double fov, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::CameraProperties)
{
    m_ui->setupUi(this);

    QObject::connect(m_ui->m_sliderFOV, SIGNAL(sliderMoved(int)),
                     this, SLOT(onSliderMoved(int)),
                     Qt::UniqueConnection);
    QObject::connect(m_ui->m_sliderFOV, SIGNAL(valueChanged(int)),
                     this, SLOT(onSliderMoved(int)),
                     Qt::UniqueConnection);

    m_ui->m_labelValue->setText(QString::number(fov));
}

CameraProperties::~CameraProperties()
{
}

void CameraProperties::onSliderMoved(int position)
{
    m_ui->m_labelValue->setText(QString::number(position));
    emit fovChanged(double(position));
}
