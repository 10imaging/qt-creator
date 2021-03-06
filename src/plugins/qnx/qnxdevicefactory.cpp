/****************************************************************************
**
** Copyright (C) 2016 BlackBerry Limited. All rights reserved.
** Contact: KDAB (info@kdab.com)
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "qnxdevicefactory.h"

#include "qnxconstants.h"
#include "qnxdevicewizard.h"
#include "qnxdevice.h"

#include <utils/qtcassert.h>
#include <utils/icon.h>

#include <QIcon>

namespace Qnx {
namespace Internal {

QnxDeviceFactory::QnxDeviceFactory()
    : ProjectExplorer::IDeviceFactory(Constants::QNX_QNX_OS_TYPE)
{
}

QString QnxDeviceFactory::displayName() const
{
    return tr("QNX Device");
}

QIcon QnxDeviceFactory::icon() const
{
    using namespace Utils;
    static const QIcon icon =
            Icon::combinedIcon({Icon({{":/qnx/images/qnxdevicesmall.png",
                                       Theme::PanelTextColorDark}}, Icon::Tint),
                                Icon({{":/qnx/images/qnxdevice.png",
                                       Theme::IconsBaseColor}})});
    return icon;
}

bool QnxDeviceFactory::canCreate() const
{
    return true;
}

ProjectExplorer::IDevice::Ptr QnxDeviceFactory::create() const
{
    QnxDeviceWizard wizard;
    if (wizard.exec() != QDialog::Accepted)
        return ProjectExplorer::IDevice::Ptr();
    return wizard.device();
}

ProjectExplorer::IDevice::Ptr QnxDeviceFactory::restore(const QVariantMap &map) const
{
    QTC_ASSERT(canRestore(map), return QnxDevice::Ptr());
    const QnxDevice::Ptr device = QnxDevice::create();
    device->fromMap(map);
    return device;
}

Core::Id QnxDeviceFactory::deviceType()
{
    return Core::Id(Constants::QNX_QNX_OS_TYPE);
}

} // namespace Internal
} // namespace Qnx
