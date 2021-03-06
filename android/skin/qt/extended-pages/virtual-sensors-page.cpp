// Copyright (C) 2015 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
#include "android/skin/qt/extended-pages/virtual-sensors-page.h"

#include "android/emulation/control/sensors_agent.h"
#include "android/hw-sensors.h"
#include <QQuaternion>

VirtualSensorsPage::VirtualSensorsPage(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::VirtualSensorsPage()),
    mSensorsAgent(nullptr)
{
    mUi->setupUi(this);
    mUi->temperatureSensorValueWidget->setRange(-273.1, 100.0);
    mUi->temperatureSensorValueWidget->setValue(25.0);
    mUi->lightSensorValueWidget->setRange(0, 40000.0);
    mUi->lightSensorValueWidget->setValue(20000.0);
    mUi->pressureSensorValueWidget->setRange(300, 1100);
    mUi->pressureSensorValueWidget->setValue(1013.25);
    mUi->humiditySensorValueWidget->setRange(0, 100);
    mUi->humiditySensorValueWidget->setValue(50);
    mUi->proximitySensorValueWidget->setRange(0, 10);
    mUi->proximitySensorValueWidget->setValue(10);
    mUi->magNorthWidget->setValidator(&mMagFieldValidator);
    mUi->magNorthWidget->setTextMargins(0, 0, 0, 4);
    mUi->magEastWidget->setValidator(&mMagFieldValidator);
    mUi->magEastWidget->setTextMargins(0, 0, 0, 4);
    mUi->magVerticalWidget->setValidator(&mMagFieldValidator);
    mUi->magVerticalWidget->setTextMargins(0, 0, 0, 4);

    updateAccelerometerValues();

    connect(mUi->magNorthWidget,
            SIGNAL(editingFinished()),
            this,
            SLOT(onMagVectorChanged()));
    connect(mUi->magEastWidget,
            SIGNAL(editingFinished()),
            this,
            SLOT(onMagVectorChanged()));
    connect(mUi->magVerticalWidget,
            SIGNAL(editingFinished()),
            this,
            SLOT(onMagVectorChanged()));
    connect(mUi->accelWidget,
            SIGNAL(rotationChanged()),
            this,
            SLOT(onPhoneRotationChanged()));
    connect(mUi->accelWidget,
            SIGNAL(positionChanged()),
            this,
            SLOT(onPhonePositionChanged()));
    connect(mUi->accelWidget,
            SIGNAL(dragStopped()),
            this,
            SLOT(onDragStopped()));
    connect(mUi->accelWidget,
            SIGNAL(dragStarted()),
            this,
            SLOT(onDragStarted()));

    connect(&mAccelerationTimer, SIGNAL(timeout()), this, SLOT(updateLinearAcceleration()));
    mAccelerationTimer.setInterval(100);
    mAccelerationTimer.stop();
}

void VirtualSensorsPage::setSensorsAgent(const QAndroidSensorsAgent* agent) {
    mSensorsAgent = agent;

    // Update the agent with current values.
    updateAccelerometerValues();
}

// Helper function
static void setSensorValue(
        const QAndroidSensorsAgent* agent,
        AndroidSensor sensor_id,
        double v1,
        double v2 = 0.0,
        double v3 = 0.0) {
    if (agent) {
        agent->setSensor(sensor_id, 
                         static_cast<float>(v1),
                         static_cast<float>(v2),
                         static_cast<float>(v3));
    }
}

void VirtualSensorsPage::on_temperatureSensorValueWidget_valueChanged(double value) {
    setSensorValue(mSensorsAgent, ANDROID_SENSOR_TEMPERATURE, value);
}

void VirtualSensorsPage::on_proximitySensorValueWidget_valueChanged(double value) {
    setSensorValue(mSensorsAgent, ANDROID_SENSOR_PROXIMITY, value);
}

void VirtualSensorsPage::on_lightSensorValueWidget_valueChanged(double value) {
    setSensorValue(mSensorsAgent, ANDROID_SENSOR_LIGHT, value);
}

void VirtualSensorsPage::on_pressureSensorValueWidget_valueChanged(double value) {
    setSensorValue(mSensorsAgent, ANDROID_SENSOR_PRESSURE, value);
}

void VirtualSensorsPage::on_humiditySensorValueWidget_valueChanged(double value) {
    setSensorValue(mSensorsAgent, ANDROID_SENSOR_HUMIDITY, value);
}

void VirtualSensorsPage::onMagVectorChanged() {
    updateAccelerometerValues();
}

void VirtualSensorsPage::onPhoneRotationChanged() {
    updateAccelerometerValues();
}

// Helper function.
static QString formatSensorValue(double value) {
    return QString("%1").arg(value, 8, 'f', 2, ' ');
}

void VirtualSensorsPage::updateAccelerometerValues() {
    // Gravity and magnetic vector in the device's frame of
    // reference.
    QVector3D gravity_vector(0.0, 9.8, 0.0);
    QVector3D magnetic_vector(
            mUi->magNorthWidget->text().toDouble(),
            mUi->magEastWidget->text().toDouble(),
            mUi->magVerticalWidget->text().toDouble());

    QQuaternion device_rotation_quat = mUi->accelWidget->rotation();

    // Gravity and magnetic vectors as observed by the device.
    // Note how we're applying the *inverse* of the transformation
    // represented by device_rotation_quat to the "absolute" coordinates
    // of the vectors.
    QVector3D device_gravity_vector =
        device_rotation_quat.conjugate().rotatedVector(gravity_vector);
    QVector3D device_magnetic_vector =
        device_rotation_quat.conjugate().rotatedVector(magnetic_vector);

    QVector3D acceleration = device_gravity_vector - mLinearAcceleration;

    // Acceleration is affected both by the gravity and linear movement of the device.
    // For now, we don't have a linear component, so just account for gravity.
    setSensorValue(mSensorsAgent,
                   ANDROID_SENSOR_ACCELERATION,
                   acceleration.x(),
                   acceleration.y(),
                   acceleration.z());

    setSensorValue(mSensorsAgent,
                   ANDROID_SENSOR_MAGNETIC_FIELD,
                   device_magnetic_vector.x(),
                   device_magnetic_vector.y(),
                   device_magnetic_vector.z());

    // Update labels with new values.
    mUi->accelerometerXLabel->setText(formatSensorValue(acceleration.x()));
    mUi->accelerometerYLabel->setText(formatSensorValue(acceleration.y()));
    mUi->accelerometerZLabel->setText(formatSensorValue(acceleration.z()));
    mUi->magnetometerNorthLabel->setText(formatSensorValue(device_magnetic_vector.x()));
    mUi->magnetometerEastLabel->setText(formatSensorValue(device_magnetic_vector.y()));
    mUi->magnetometerVerticalLabel->setText(formatSensorValue(device_magnetic_vector.z()));
}

void VirtualSensorsPage::onPhonePositionChanged() {
    const QVector2D& pos = mUi->accelWidget->position();
    mCurrentPosition = QVector3D(pos.x(), pos.y(), 0.0);
}

void VirtualSensorsPage::updateLinearAcceleration() {
    static const float k = 100.0;
    static const float mass = 1.0;
    static const float meters_per_unit = 0.0254;

    QVector3D delta =
        mUi->accelWidget->rotation().conjugate().rotatedVector(
            meters_per_unit * (mCurrentPosition - mPrevPosition));
    mLinearAcceleration = delta * k / mass;
    mPrevPosition = mCurrentPosition;
    updateAccelerometerValues();
}

void VirtualSensorsPage::on_accelModeRotate_toggled() {
    if (mUi->accelModeRotate->isChecked()) {
        mUi->accelWidget->setOperationMode(
            Accelerometer3DWidget::OperationMode::Rotate);
    }
}

void VirtualSensorsPage::on_accelModeMove_toggled() {
    if (mUi->accelModeMove->isChecked()) {
        mUi->accelWidget->setOperationMode(
            Accelerometer3DWidget::OperationMode::Move);
    }
}
