// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Exercise the compiled plugin through KDecoration, using synthetic windows and
// a windowless Qt process. No installed decoration or live desktop is touched.

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationSettings>
#include <KDecoration3/Private/DecoratedWindowPrivate>
#include <KDecoration3/Private/DecorationBridge>
#include <KDecoration3/Private/DecorationSettingsPrivate>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>
#include <QTemporaryDir>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
using namespace KDecoration3;
using Type = DecorationButtonType;
static void require(bool condition, const QString &message)
{
    if (!condition)
        throw std::runtime_error(message.toStdString());
}
static bool equal(qreal a, qreal b)
{
    return std::abs(a - b) < 0.00001;
}
struct State {
    bool maximized = false;
    bool shaded = false;
    Qt::Edges edges;
    qreal scale = 1;
    qreal width = 800;
    int fontSize = 10;
    BorderSize border = BorderSize::None;
    bool alpha = true;
    bool active = true;
    bool drawBorderOnMaximized = false;
    bool alternateLayout = false;
    int closeRequests = 0;
};
class Client : public DecoratedWindowPrivate
{
public:
    Client(DecoratedWindow *w, Decoration *d, State &state)
        : DecoratedWindowPrivate(w, d)
        , s(state)
    {
    }
    State &s;
    bool isActive() const override
    {
        return s.active;
    }
    QString caption() const override
    {
        return QStringLiteral("Geometry check");
    }
    bool isOnAllDesktops() const override
    {
        return false;
    }
    bool isShaded() const override
    {
        return s.shaded;
    }
    QIcon icon() const override
    {
        return {};
    }
    bool isMaximized() const override
    {
        return s.maximized;
    }
    bool isMaximizedHorizontally() const override
    {
        return s.maximized;
    }
    bool isMaximizedVertically() const override
    {
        return s.maximized;
    }
    bool isKeepAbove() const override
    {
        return false;
    }
    bool isKeepBelow() const override
    {
        return false;
    }
    bool isCloseable() const override
    {
        return true;
    }
    bool isMaximizeable() const override
    {
        return true;
    }
    bool isMinimizeable() const override
    {
        return true;
    }
    bool providesContextHelp() const override
    {
        return false;
    }
    bool isModal() const override
    {
        return false;
    }
    bool isShadeable() const override
    {
        return true;
    }
    bool isMoveable() const override
    {
        return true;
    }
    bool isResizeable() const override
    {
        return true;
    }
    qreal width() const override
    {
        return s.width;
    }
    qreal height() const override
    {
        return 500;
    }
    QSizeF size() const override
    {
        return {width(), height()};
    }
    QPalette palette() const override
    {
        return QPalette(Qt::white);
    }
    Qt::Edges adjacentScreenEdges() const override
    {
        return s.edges;
    }
    qreal scale() const override
    {
        return s.scale;
    }
    qreal nextScale() const override
    {
        return s.scale;
    }
    void requestShowToolTip(const QString &) override
    {
    }
    void requestHideToolTip() override
    {
    }
    void requestClose() override
    {
        ++s.closeRequests;
    }
    void requestToggleMaximization(Qt::MouseButtons) override
    {
    }
    void requestMinimize() override
    {
    }
    void requestContextHelp() override
    {
    }
    void requestToggleOnAllDesktops() override
    {
    }
    void requestToggleShade() override
    {
    }
    void requestToggleKeepAbove() override
    {
    }
    void requestToggleKeepBelow() override
    {
    }
    void requestShowWindowMenu(const QRect &) override
    {
    }
    QString windowClass() const override
    {
        return QStringLiteral("geometry-check");
    }
    bool hasApplicationMenu() const override
    {
        return false;
    }
    bool isApplicationMenuActive() const override
    {
        return false;
    }
    void showApplicationMenu(int) override
    {
    }
    void requestShowApplicationMenu(const QRect &, int) override
    {
    }
    QColor color(ColorGroup group, ColorRole role) const override
    {
        if (group == ColorGroup::Warning)
            return QColor(Qt::red);
        return role == ColorRole::Foreground ? QColor(Qt::black) : QColor(Qt::white);
    }
};
class Settings : public DecorationSettingsPrivate
{
public:
    Settings(DecorationSettings *p, State &state)
        : DecorationSettingsPrivate(p)
        , s(state)
    {
    }
    State &s;
    bool isOnAllDesktopsAvailable() const override
    {
        return true;
    }
    bool isAlphaChannelSupported() const override
    {
        return s.alpha;
    }
    bool isCloseOnDoubleClickOnMenu() const override
    {
        return false;
    }
    QList<Type> decorationButtonsLeft() const override
    {
        return {Type::OnAllDesktops};
    }
    QList<Type> decorationButtonsRight() const override
    {
        if (s.alternateLayout)
            return {Type::Close, Type::Maximize, Type::Minimize};
        return {Type::Minimize, Type::Maximize, Type::Close};
    }
    BorderSize borderSize() const override
    {
        return s.border;
    }
    QFont font() const override
    {
        return QFont(QStringLiteral("DejaVu Sans"), s.fontSize);
    }
};
class Bridge : public DecorationBridge
{
public:
    explicit Bridge(State &state)
        : s(state)
    {
    }
    State &s;
    std::unique_ptr<DecoratedWindowPrivate> createClient(DecoratedWindow *w, Decoration *d) override
    {
        return std::make_unique<Client>(w, d, s);
    }
    std::unique_ptr<DecorationSettingsPrivate> settings(DecorationSettings *p) override
    {
        return std::make_unique<Settings>(p, s);
    }
};
static void hover(Decoration *d, QPointF point)
{
    QHoverEvent event(QEvent::HoverMove, point, point, QPointF(-1, -1));
    QCoreApplication::sendEvent(d, &event);
}
static QImage renderButton(DecorationButton *button)
{
    QImage image(256, 256, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.translate(QPointF(128, 128) - button->geometry().center());
    button->paint(&painter, button->geometry());
    return image;
}
static QImage renderDecoration(Decoration *d, QColor backdrop = Qt::transparent, QPointF offset = {}, const QRectF &clip = {})
{
    const qreal scale = d->window()->scale();
    QImage image(int(std::ceil((d->size().width() + offset.x()) * scale)) + 4,
                 int(std::ceil((d->borderTop() + offset.y()) * scale)) + 8,
                 QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(scale);
    image.fill(backdrop);
    QPainter painter(&image);
    painter.translate(offset);
    if (!clip.isEmpty())
        painter.setClipRect(clip);
    d->paint(&painter, QRectF(image.rect()));
    return image;
}
static void mouse(Decoration *d, QEvent::Type type, QPointF p)
{
    QMouseEvent e(type, p, p, Qt::LeftButton, type == QEvent::MouseButtonPress ? Qt::LeftButton : Qt::NoButton, Qt::NoModifier);
    e.setAccepted(false);
    QCoreApplication::sendEvent(d, &e);
    QCoreApplication::processEvents();
}
static void sameAlpha(const QImage &a, const QImage &b, const QString &key)
{
    require(a.size() == b.size(), key + " image size");
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
            if (qAlpha(a.pixel(x, y)) != qAlpha(b.pixel(x, y)))
                require(false, key + QStringLiteral(" alpha changed at %1,%2: %3 -> %4").arg(x).arg(y).arg(qAlpha(a.pixel(x, y))).arg(qAlpha(b.pixel(x, y))));
}
static void checkCorners(Decoration *d, DecorationButton *outer, State &s, const QString &key)
{
    const qreal w = d->size().width(), h = d->borderTop();
    const bool rounded = s.alpha && (!s.maximized || s.drawBorderOnMaximized);
    hover(d, {w / 2, h / 2});
    const QImage idle = renderDecoration(d);
    hover(d, outer->geometry().center());
    const QImage hovered = renderDecoration(d);
    sameAlpha(idle, hovered, key + " hovered");
    const QColor backdrop(50, 80, 120);
    const QImage preview = renderDecoration(d, backdrop);
    for (int y = 0; y < int(h * s.scale); ++y)
        for (int x = 0; x < idle.width(); ++x)
            if (qAlpha(idle.pixel(x, y)) == 0)
                require(preview.pixelColor(x, y) == backdrop, key + " hover changed backdrop outside titlebar outline");
    // KWin clips partial damage; the preview also translates for shadow padding.
    const QRectF clip(w - h - 2, 0, h + 2, h);
    const QPointF offset(7.3, 9.7);
    const QImage whole = renderDecoration(d, backdrop, offset);
    const QImage partial = renderDecoration(d, backdrop, offset, clip);
    for (int y = int(std::ceil(offset.y() * s.scale)) + 1; y < int(std::floor((offset.y() + h) * s.scale)) - 1; ++y)
        for (int x = int(std::ceil((offset.x() + clip.left()) * s.scale)) + 1; x < int(std::floor((offset.x() + w) * s.scale)) - 1; ++x)
            require(whole.pixel(x, y) == partial.pixel(x, y), key + " translated partial repaint differs");
    mouse(d, QEvent::MouseButtonPress, outer->geometry().center());
    const QImage pressed = renderDecoration(d);
    sameAlpha(idle, pressed, key + " pressed");
    mouse(d, QEvent::MouseButtonRelease, {w / 2, h + 1});

    const int before = s.closeRequests;
    QList<QPointF> corners{{w - 0.01, 0.01}};
    if (s.shaded)
        corners.append({w - 0.01, h - 0.01});
    for (const auto p : corners) {
        hover(d, p);
        require(outer->isHovered() != rounded, key + " corner hover membership");
        mouse(d, QEvent::MouseButtonPress, p);
        require(outer->isPressed() != rounded, key + " corner press membership");
        mouse(d, QEvent::MouseButtonRelease, p);
        if (rounded)
            require(s.closeRequests == before, key + " corner activated close");
    }
    // Compare actual pointer routing to fully opaque/transparent pixels of the
    // rendered corner, independently of the implementation's path construction.
    const int span = int(std::ceil(3 * d->settings()->smallSpacing() * s.scale)) + 2;
    const int x0 = int(std::floor(w * s.scale));
    for (int y = 0; y < span; ++y)
        for (int x = x0 - span; x < x0; ++x) {
            const QPointF p((x + .5) / s.scale, (y + .5) / s.scale);
            if (!outer->geometry().contains(p))
                continue;
            const int alpha = qAlpha(idle.pixel(x, y));
            if (alpha != 0 && alpha != 255)
                continue; // Antialiased coverage is not a binary hit area.
            hover(d, p);
            require(outer->isHovered() == bool(alpha),
                    key + QStringLiteral(" painted corner and hover disagree at %1,%2 alpha=%3").arg(p.x()).arg(p.y()).arg(alpha));
            mouse(d, QEvent::MouseButtonPress, p);
            require(outer->isPressed() == bool(alpha), key + " painted corner and press disagree");
            mouse(d, QEvent::MouseButtonRelease, {w / 2, h + 1});
        }
    hover(d, {w / 2, h / 2});
}
static QJsonObject iconFingerprint(DecorationButton *button)
{
    const QImage image = renderButton(button);
    QRect bounds;
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            if (qAlpha(image.pixel(x, y)))
                bounds = bounds.united(QRect(x, y, 1, 1));
    require(!bounds.isEmpty(), QStringLiteral("Missing button symbol"));
    const QImage crop = image.copy(bounds);
    const QByteArray bytes(reinterpret_cast<const char *>(crop.constBits()), crop.sizeInBytes());
    return {{"width", bounds.width()},
            {"height", bounds.height()},
            {"hash", QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex())}};
}
int main(int argc, char **argv)
{
    QTemporaryDir temp;
    if (!temp.isValid()) {
        std::cerr << "Could not create isolated test directory\n";
        return 2;
    }
    const QByteArray root = temp.path().toUtf8();
    qputenv("XDG_CONFIG_HOME", root + "/config");
    qputenv("XDG_CONFIG_DIRS", root + "/config-dirs");
    qputenv("XDG_CACHE_HOME", root + "/cache");
    qputenv("XDG_DATA_HOME", root + "/data");
    qputenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/nonexistent-breezesquared-test-bus");
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    QDir().mkpath(temp.path() + "/config");
    QGuiApplication app(argc, argv); // No windows are created or shown.
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("BreezeSquared decoration regression checks"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("plugin"), QStringLiteral("Path to a compiled decoration plugin"));
    parser.addPositionalArgument(QStringLiteral("button-size"), QStringLiteral("ButtonTiny, ButtonSmall, ButtonDefault, ButtonLarge or ButtonVeryLarge"));
    parser.addOptions({
        {QStringLiteral("baseline"), QStringLiteral("Record upstream geometry and icons without asserting fork behavior")},
        {QStringLiteral("no-alpha"), QStringLiteral("Simulate a decoration without alpha-channel support")},
        {QStringLiteral("inactive"), QStringLiteral("Simulate inactive windows")},
        {QStringLiteral("border-on-maximized"), QStringLiteral("Enable borders on maximized windows")},
        {QStringLiteral("alternate-layout"), QStringLiteral("Place Minimize at the right edge")},
        {QStringLiteral("gradient"), QStringLiteral("Enable the title-bar background gradient")},
        {QStringLiteral("report"), QStringLiteral("Write the JSON report to this file"), QStringLiteral("file")},
    });
    parser.process(app);
    const QStringList positional = parser.positionalArguments();
    if (positional.size() != 2)
        parser.showHelp(2);
    const QStringList sizes{QStringLiteral("ButtonTiny"),
                            QStringLiteral("ButtonSmall"),
                            QStringLiteral("ButtonDefault"),
                            QStringLiteral("ButtonLarge"),
                            QStringLiteral("ButtonVeryLarge")};
    const bool baseline = parser.isSet(QStringLiteral("baseline"));
    try {
        require(sizes.contains(positional.at(1)), QStringLiteral("Unknown button size"));
        require(QFontDatabase::hasFamily(QStringLiteral("DejaVu Sans")), QStringLiteral("Install the DejaVu Sans font to run these checks"));
        QByteArray configData("[Common]\nShadowSize=ShadowNone\nOutlineEnabled=false\n[Windeco]\nButtonSize=");
        configData += positional.at(1).toUtf8() + '\n';
        if (parser.isSet(QStringLiteral("border-on-maximized")))
            configData += "DrawBorderOnMaximizedWindows=true\n";
        if (parser.isSet(QStringLiteral("gradient")))
            configData += "DrawBackgroundGradient=true\n";
        // Both names receive identical settings so comparison with untouched
        // upstream Breeze also covers every button-size preset.
        for (const auto &name : {QStringLiteral("breezesquaredrc"), QStringLiteral("breezerc")}) {
            QFile config(temp.path() + "/config/" + name);
            require(config.open(QIODevice::WriteOnly), QStringLiteral("Could not write isolated settings"));
            require(config.write(configData) == configData.size(), QStringLiteral("Could not finish isolated settings"));
        }
        const QFileInfo pluginFile(positional.at(0));
        require(pluginFile.isFile(), QStringLiteral("Plugin does not exist: ") + pluginFile.absoluteFilePath());
        auto loaded = KPluginFactory::loadFactory(KPluginMetaData(pluginFile.absoluteFilePath()));
        require(bool(loaded), loaded.errorText);
        QJsonArray rows;
        for (int fontSize : {8, 10, 14})
            for (qreal scale : {1., 1.25, 1.5, 2.})
                for (auto border : {BorderSize::None, BorderSize::Tiny})
                    for (int stateIndex = 0; stateIndex < 6; ++stateIndex) {
                        State s;
                        s.fontSize = fontSize;
                        s.scale = scale;
                        s.border = border;
                        s.alpha = !parser.isSet(QStringLiteral("no-alpha"));
                        s.active = !parser.isSet(QStringLiteral("inactive"));
                        s.drawBorderOnMaximized = parser.isSet(QStringLiteral("border-on-maximized"));
                        s.alternateLayout = parser.isSet(QStringLiteral("alternate-layout"));
                        s.maximized = stateIndex == 1;
                        if (stateIndex == 2)
                            s.edges = Qt::TopEdge | Qt::LeftEdge;
                        if (stateIndex == 3)
                            s.edges = Qt::TopEdge | Qt::RightEdge;
                        if (stateIndex == 4)
                            s.edges = Qt::BottomEdge | Qt::RightEdge;
                        if (stateIndex == 5)
                            s.shaded = true;
                        Bridge bridge(s);
                        auto settings = std::make_shared<DecorationSettings>(&bridge);
                        QVariantMap args{{QStringLiteral("bridge"), QVariant::fromValue<DecorationBridge *>(&bridge)}};
                        std::unique_ptr<Decoration> d(loaded.plugin->create<Decoration>(nullptr, {args}));
                        require(bool(d), QStringLiteral("Could not instantiate decoration"));
                        d->setSettings(settings);
                        d->create();
                        QObject::connect(d.get(), &Decoration::nextStateChanged, d.get(), [&](auto state) {
                            d->apply(state->clone());
                        });
                        require(d->init(), QStringLiteral("Decoration init failed"));
                        QCoreApplication::processEvents();
                        const auto groups = d->findChildren<DecorationButtonGroup *>();
                        const qreal height = d->borderTop();
                        const QRectF originalTitleBar = d->titleBar();
                        const QString key = QStringLiteral("font=%1 scale=%2 border=%3 state=%4").arg(fontSize).arg(scale).arg(int(border)).arg(stateIndex);
                        QJsonObject row{
                            {"case", key},
                            {"height", height},
                            {"titleBar", QJsonArray{originalTitleBar.x(), originalTitleBar.y(), originalTitleBar.width(), originalTitleBar.height()}}};
                        DecorationButton *close = nullptr;
                        for (auto group : groups) {
                            if (!baseline)
                                require(equal(group->spacing(), 0), key + " spacing");
                            if (!baseline) {
                                const QRectF groupRect = group->geometry();
                                for (qreal x = std::ceil(groupRect.left()); x < groupRect.right(); x += 1.) {
                                    hover(d.get(), QPointF(x, groupRect.center().y()));
                                    int hovered = 0;
                                    for (auto button : group->buttons())
                                        hovered += button->isHovered();
                                    require(hovered == 1, key + QStringLiteral(" boundary hover at %1: %2 buttons").arg(x).arg(hovered));
                                }
                                hover(d.get(), QPointF(d->size().width() / 2, height - 1));
                            }
                            qreal previousRight = -1;
                            for (auto b : group->buttons()) {
                                if (b->type() == Type::Close)
                                    close = b;
                                row[QString::number(int(b->type()))] = iconFingerprint(b);
                                if (baseline)
                                    continue;
                                const auto rect = b->geometry();
                                require(equal(rect.width(), height) && equal(rect.height(), height) && equal(rect.y(), 0), key + " square/full-height");
                                if (previousRight >= 0)
                                    require(equal(previousRight, rect.left()), key + " adjacent geometry");
                                previousRight = rect.right();
                                for (const QPointF &p : QList<QPointF>{{rect.center().x(), 0},
                                                                       {rect.center().x(), height - 0.01},
                                                                       {rect.left() + 0.01, height / 2},
                                                                       {rect.right() - 0.01, height / 2}}) {
                                    hover(d.get(), p);
                                    require(b->isHovered(), key + " top/bottom hover");
                                    require(d->sectionUnderMouse() == Qt::TitleBarArea, key + " resize cursor over button");
                                    QMouseEvent press(QEvent::MouseButtonPress, p, p, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                                    press.setAccepted(false);
                                    QCoreApplication::sendEvent(d.get(), &press);
                                    require(press.isAccepted() && b->isPressed(), key + " top/bottom press");
                                    QMouseEvent release(QEvent::MouseButtonRelease, p, p, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                                    QCoreApplication::sendEvent(d.get(), &release);
                                    require(!b->isPressed(), key + " release");
                                    const QImage highlight = renderButton(b);
                                    require(qAlpha(highlight.pixel(128, int(std::ceil(128 - height / 2)))) > 0, key + " highlight top");
                                    require(qAlpha(highlight.pixel(128, int(std::floor(128 + height / 2)) - 1)) > 0, key + " highlight bottom");
                                }
                                hover(d.get(), QPointF(d->size().width() / 2, height - 1));
                            }
                        }
                        QCoreApplication::processEvents();
                        require(close, key + " close missing");
                        if (!baseline) {
                            require(s.closeRequests == 4, key + QStringLiteral(" close action dispatch: %1").arg(s.closeRequests));
                            hover(d.get(), close->geometry().center());
                            const QPointF pressPoint = close->geometry().center();
                            QMouseEvent cancelledPress(QEvent::MouseButtonPress, pressPoint, pressPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                            QCoreApplication::sendEvent(d.get(), &cancelledPress);
                            const QPointF outside(d->size().width() / 2, height + 1);
                            hover(d.get(), outside);
                            QMouseEvent cancelledRelease(QEvent::MouseButtonRelease, outside, outside, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                            QCoreApplication::sendEvent(d.get(), &cancelledRelease);
                            QCoreApplication::processEvents();
                            require(!close->isPressed() && s.closeRequests == 4, key + " release outside must cancel click");
                            const auto outer = groups.back()->buttons().back();
                            require(equal(outer->geometry().right(), d->size().width()), key + " right outer edge");
                            const auto left = groups.front()->buttons().front();
                            if (!s.drawBorderOnMaximized && (s.maximized || s.edges.testFlag(Qt::LeftEdge)))
                                require(equal(left->geometry().left(), 0), key + " left screen edge");
                            if (s.drawBorderOnMaximized || (!s.maximized && !s.edges.testFlag(Qt::TopEdge))) {
                                hover(d.get(), QPointF(d->size().width() / 2, 0));
                                require(d->sectionUnderMouse() == Qt::TopSection, key + " top resizing elsewhere");
                            }
                            require(d->titleBar() == originalTitleBar && equal(d->borderTop(), height), key + " titlebar changed");
                            s.width = 900;
                            Q_EMIT d->window()->widthChanged(s.width);
                            require(equal(outer->geometry().right(), d->size().width()), key + " resized window alignment");
                            checkCorners(d.get(), outer, s, key);
                        }
                        if (!baseline) {
                            const auto newState = d->currentState()->clone();
                            QMarginsF borders = newState->borders();
                            borders.setTop(height + 3.2);
                            newState->setBorders(borders);
                            d->apply(newState);
                            for (auto group : groups)
                                for (auto b : group->buttons()) {
                                    require(equal(b->geometry().height(), height + 3.2) && equal(b->geometry().width(), height + 3.2),
                                            key + " border-height update");
                                }
                        }
                        rows.append(row);
                    }
        const QByteArray report = QJsonDocument(rows).toJson(QJsonDocument::Indented);
        if (parser.isSet(QStringLiteral("report"))) {
            QFile output(parser.value(QStringLiteral("report")));
            require(output.open(QIODevice::WriteOnly), QStringLiteral("Could not open report file: ") + output.fileName());
            require(output.write(report) == report.size(), QStringLiteral("Could not write report"));
            std::cout << rows.size() << " configurations " << (baseline ? "recorded" : "passed") << '\n';
        } else {
            std::cout << report.constData();
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
