/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "BlendSpaceNodeWidget.h"
#include <MCore/Source/Algorithms.h>


namespace EMStudio
{

// Current sample point
const float     BlendSpaceNodeWidget::s_currentSamplePointWidth         = 6.0f;
const QColor    BlendSpaceNodeWidget::s_currentSamplePointColor         = QColor(0xCF, 0x02, 0x1B);

// Motion points
const float     BlendSpaceNodeWidget::s_motionPointWidthMin      = 5.0f;
const float     BlendSpaceNodeWidget::s_motionPointWidthMax      = 10.0f;
const float     BlendSpaceNodeWidget::s_motionPointAlphaMin      = 1.0f;
const float     BlendSpaceNodeWidget::s_motionPointAlphaMax      = 1.0f;
const QColor    BlendSpaceNodeWidget::s_motionPointColor         = QColor(245, 166, 35);


BlendSpaceNodeWidget::BlendSpaceNodeWidget()
{
}


BlendSpaceNodeWidget::~BlendSpaceNodeWidget()
{
}


void BlendSpaceNodeWidget::RenderCurrentSamplePoint(QPainter& painter, const QPointF& samplePoint)
{
    RenderCircle(painter, samplePoint, s_currentSamplePointColor, s_currentSamplePointWidth);
}


void BlendSpaceNodeWidget::RenderSampledMotionPoint(QPainter& painter, const QPointF& point, float weight)
{
    const float alpha = MCore::LinearInterpolate<float>(s_motionPointAlphaMin, s_motionPointAlphaMax, weight);
    const float size = MCore::LinearInterpolate<float>(s_motionPointWidthMin, s_motionPointWidthMax, weight);
    
    QColor color = s_motionPointColor;
    color.setAlphaF(alpha);

    // Render the slightly transparent circle background.
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(point, size, size);

    // Render the fully opaque border
    painter.setPen(s_motionPointColor);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(point, size, size);
}


void BlendSpaceNodeWidget::RenderCircle(QPainter& painter, const QPointF& point, const QColor& color, float size)
{
    painter.setPen(color);
    painter.setBrush(color);

    painter.drawEllipse(point, size, size);
}


void BlendSpaceNodeWidget::RenderTextBox(QPainter& painter, const QPointF& point, const QString& text)
{
    QFont font;
    font.setPointSizeF(8.0f);

    painter.setFont(font);

    QFontMetrics fontMetrics(font);

    const int boxWidth = fontMetrics.width(text);
    const int boxHeight = fontMetrics.height();

    const int halfBoxWidth = static_cast<int>(static_cast<float>(boxWidth) * 0.5f + 0.5f);
    const int halfBoxHeight = static_cast<int>(static_cast<float>(boxHeight) * 0.5f + 0.5f);

    QRect rect;
    rect.setTop(point.y() - halfBoxHeight);
    rect.setLeft(point.x() - halfBoxWidth);
    rect.setBottom(point.y() + halfBoxHeight);
    rect.setRight(point.x() + halfBoxWidth);
    
    painter.setPen(Qt::NoPen);
    QColor semiTransparentColor = s_currentSamplePointColor;
    semiTransparentColor.setAlphaF(0.25f);
    painter.setBrush(semiTransparentColor);
    painter.drawRect(rect);

    painter.setPen(s_currentSamplePointColor);
    semiTransparentColor.setAlphaF(0.5f);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect);

    painter.setPen(s_currentSamplePointColor);
    painter.setBrush(s_currentSamplePointColor);
    painter.drawText(rect, Qt::AlignCenter, text);
}

} // namespace EMStudio