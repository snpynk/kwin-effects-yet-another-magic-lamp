/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "YetAnotherMagicLampEffect.h"
#include <qmargins.h>
using namespace std::chrono_literals;
// Own
#include "Model.h"
#include "effect/effecthandler.h"
#include "scene/windowitem.h"

// Auto-generated
#include "YetAnotherMagicLampConfig.h"

// std
#include <cmath>

enum ShapeCurve {
    Linear = 0,
    Quad = 1,
    Cubic = 2,
    Quart = 3,
    Quint = 4,
    Sine = 5,
    Circ = 6,
    Bounce = 7,
    Bezier = 8
};

YetAnotherMagicLampEffect::YetAnotherMagicLampEffect()
: m_taskFrame(new KSvg::FrameSvg(this))
{
    reconfigure(ReconfigureAll);

    connect(KWin::effects, &KWin::EffectsHandler::windowAdded,
        this, &YetAnotherMagicLampEffect::slotWindowAdded);
    connect(KWin::effects, &KWin::EffectsHandler::windowDeleted,
        this, &YetAnotherMagicLampEffect::slotWindowDeleted);
    connect(KWin::effects, &KWin::EffectsHandler::activeFullScreenEffectChanged,
        this, &YetAnotherMagicLampEffect::slotActiveFullScreenEffectChanged);

    const auto windows = KWin::effects->stackingOrder();
    for (KWin::EffectWindow* window : windows) {
        YetAnotherMagicLampEffect::slotWindowAdded(window);
    }

    setVertexSnappingMode(KWin::RenderGeometry::VertexSnappingMode::None);

		m_taskFrame->setImagePath(QStringLiteral("widgets/background"));
}

YetAnotherMagicLampEffect::~YetAnotherMagicLampEffect()
{
}

void YetAnotherMagicLampEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    YetAnotherMagicLampConfig::self()->read();

    QEasingCurve curve;
    const auto shapeCurve = static_cast<ShapeCurve>(YetAnotherMagicLampConfig::shapeCurve());
    switch (shapeCurve) {
    case ShapeCurve::Linear:
        curve.setType(QEasingCurve::Linear);
        break;

    case ShapeCurve::Quad:
        curve.setType(QEasingCurve::InOutQuad);
        break;

    case ShapeCurve::Cubic:
        curve.setType(QEasingCurve::InOutCubic);
        break;

    case ShapeCurve::Quart:
        curve.setType(QEasingCurve::InOutQuart);
        break;

    case ShapeCurve::Quint:
        curve.setType(QEasingCurve::InOutQuint);
        break;

    case ShapeCurve::Sine:
        curve.setType(QEasingCurve::InOutSine);
        break;

    case ShapeCurve::Circ:
        curve.setType(QEasingCurve::InOutCirc);
        break;

    case ShapeCurve::Bounce:
        curve.setType(QEasingCurve::InOutBounce);
        break;

    case ShapeCurve::Bezier:
        // With the cubic bezier curve, "0" corresponds to the furtherst edge
        // of a window, "1" corresponds to the closest edge.
        curve.setType(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.3, 0.0),
            QPointF(0.7, 1.0),
            QPointF(1.0, 1.0));
        break;
    default:
        // Fallback to the sine curve.
        curve.setType(QEasingCurve::InOutSine);
        break;
    }
    m_modelParameters.shapeCurve = curve;

    const std::chrono::milliseconds baseDuration = std::chrono::milliseconds(animationTime<YetAnotherMagicLampConfig>(250ms));
    m_modelParameters.squashDuration = std::chrono::milliseconds(baseDuration);
    m_modelParameters.stretchDuration = std::chrono::milliseconds(qMax(qRound(baseDuration.count() * 0.7), 1));
    m_modelParameters.bumpDuration = std::chrono::milliseconds(baseDuration);
    m_modelParameters.shapeFactor = YetAnotherMagicLampConfig::initialShapeFactor();
    m_modelParameters.bumpDistance = YetAnotherMagicLampConfig::maxBumpDistance();

    m_gridResolution = YetAnotherMagicLampConfig::gridResolution();
}

void YetAnotherMagicLampEffect::prePaintScreen(KWin::ScreenPrePaintData& data, std::chrono::milliseconds presentTime)
{
    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    KWin::effects->prePaintScreen(data, presentTime);
}

void YetAnotherMagicLampEffect::prePaintWindow(KWin::RenderView* view, KWin::EffectWindow* w, KWin::WindowPrePaintData& data, std::chrono::milliseconds presentTime)
{
    auto animationIt = m_animations.find(w);
    if (animationIt != m_animations.end()) {
        (animationIt)->model.advance(presentTime);
        data.setTransformed();
    }

    KWin::effects->prePaintWindow(view, w, data, presentTime);
}

void YetAnotherMagicLampEffect::postPaintScreen()
{
    for (auto it = m_animations.begin(); it != m_animations.end();) {
        if (it->model.done()) {
            unredirect(it.key());
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }

    KWin::effects->addRepaintFull();
    KWin::effects->postPaintScreen();
}

void YetAnotherMagicLampEffect::paintWindow(const KWin::RenderTarget& renderTarget, const KWin::RenderViewport& viewport, KWin::EffectWindow* w, int mask, const KWin::Region& deviceRegion, KWin::WindowPaintData& data)
{
    KWin::Region clip = deviceRegion;

    auto it = m_animations.constFind(w);
    if (it != m_animations.constEnd()) {
        if (it->model.needsClip()) {
            clip = it->model.clipRegion();
        }
    }

    KWin::effects->paintWindow(renderTarget, viewport, w, mask, deviceRegion, data);
}

void YetAnotherMagicLampEffect::apply(KWin::EffectWindow* window, int mask, KWin::WindowPaintData& data, KWin::WindowQuadList& quads)
{
    Q_UNUSED(mask)
    Q_UNUSED(data)

    auto it = m_animations.constFind(window);
    if (it == m_animations.constEnd()) {
        return;
    }

    quads = quads.makeGrid(m_gridResolution);
    (*it).model.apply(quads, data);
}

bool YetAnotherMagicLampEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool YetAnotherMagicLampEffect::supported()
{
    return OffscreenEffect::supported() && KWin::effects->animationsSupported();
}

void YetAnotherMagicLampEffect::slotWindowMinimized(KWin::EffectWindow* w)
{
    if (KWin::effects->activeFullScreenEffect()) {
        return;
    }

    const QRectF iconRect = w->iconGeometry();
    if (!iconRect.isValid()) {
        return;
    }

    AnimationData& data = m_animations[w];
    data.model.setWindow(w);
    data.model.setParameters(m_modelParameters);
		data.model.setIconMargins(iconMargins());
    data.model.start(Model::AnimationKind::Minimize);
    data.visibleRef = KWin::EffectWindowVisibleRef(w, KWin::EffectWindow::PAINT_DISABLED_BY_MINIMIZE);

    redirect(w);
    KWin::effects->addRepaintFull();
}

void YetAnotherMagicLampEffect::slotWindowUnminimized(KWin::EffectWindow* w)
{
    if (KWin::effects->activeFullScreenEffect()) {
        return;
    }

    const QRectF iconRect = w->iconGeometry();
    if (!iconRect.isValid()) {
        return;
    }

    AnimationData& data = m_animations[w];
    data.model.setWindow(w);
    data.model.setParameters(m_modelParameters);
		data.model.setIconMargins(iconMargins());
    data.model.start(Model::AnimationKind::Unminimize);

    redirect(w);

    KWin::effects->addRepaintFull();
}

void YetAnotherMagicLampEffect::slotWindowDeleted(KWin::EffectWindow* w)
{
    m_animations.remove(w);
}

void YetAnotherMagicLampEffect::slotWindowAdded(KWin::EffectWindow* w)
{
    connect(w, &KWin::EffectWindow::minimizedChanged, this, [this, w]() {
        if (w->isMinimized()) {
            slotWindowMinimized(w);
        } else {
            slotWindowUnminimized(w);
        }
    });
}

void YetAnotherMagicLampEffect::slotActiveFullScreenEffectChanged()
{
    if (KWin::effects->activeFullScreenEffect() != nullptr) {
        for (auto it = m_animations.constBegin(); it != m_animations.constEnd(); ++it) {
            unredirect(it.key());
        }
        m_animations.clear();
    }
}

QMarginsF YetAnotherMagicLampEffect::iconMargins()
{
		qreal leftMargin = m_taskFrame->marginSize(KSvg::FrameSvg::LeftMargin) * .5;
		qreal topMargin = m_taskFrame->marginSize(KSvg::FrameSvg::TopMargin) * .5;

		return QMarginsF(
			leftMargin,
			topMargin, 
			leftMargin, 
			topMargin
		);
}
