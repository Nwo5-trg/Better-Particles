#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

auto mod = Mod::get();

const auto emptyCCC4F = ccc4f(0.0f, 0.0f, 0.0f, 0.0f);

class $modify(LevelEditorLayer) {
    struct Fields {
        CCDrawNode* particleBoxDrawNode;

        ccColor4F col1 = ccc4FFromccc4B(mod->getSettingValue<cocos2d::ccColor4B>("col1"));
        ccColor4F col2 = ccc4FFromccc4B(mod->getSettingValue<cocos2d::ccColor4B>("col2"));
        ccColor4F col3 = ccc4FFromccc4B(mod->getSettingValue<cocos2d::ccColor4B>("col3"));
        float thickness = mod->getSettingValue<double>("thickness");
        float centerLineThickness =  mod->getSettingValue<double>("center-line-thickness");
        float fill =  mod->getSettingValue<double>("fill");
        bool fillForRect = mod->getSettingValue<bool>("fill-for-rect");
        bool linestoCenter = mod->getSettingValue<bool>("lines-to-center");
        bool col1ForCenterLines = mod->getSettingValue<bool>("col1-for-center-lines");
        bool hideOnNonLayer = mod->getSettingValue<bool>("hide-on-different-layer");
    };

    bool init(GJGameLevel* p0, bool p1) {
        if (!LevelEditorLayer::init(p0, p1)) return false;
        auto particleBoxDrawNode = CCDrawNode::create();
        particleBoxDrawNode->setPosition(ccp(0.0f, 0.0f));
        particleBoxDrawNode->setZOrder(1401);
        particleBoxDrawNode->setID("particle-box-draw-node");
        particleBoxDrawNode->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        m_objectLayer->addChild(particleBoxDrawNode);
        m_fields->particleBoxDrawNode = particleBoxDrawNode;
        return true;
    }

    void updateDebugDraw() {
        LevelEditorLayer::updateDebugDraw();
        updateParticleRender();
    }

    void updateParticleRender() {
        if (!m_particleObjects || m_particleObjects->count() == 0) return;
        auto fields = m_fields.self();
        auto drawNode = fields->particleBoxDrawNode;
        if (!fields->particleBoxDrawNode) return;
        drawNode->clear();
        if (m_hideParticleIcons || m_alwaysHideParticleIcons) return;
        auto col1 = fields->col1;
        auto col2 = fields->col2;
        auto col3 = fields->col1ForCenterLines ? col1 : fields->col3;
        auto zoom = m_objectLayer->getScale();
        auto thickness = fields->thickness / zoom;
        auto centerLineThickness = fields->centerLineThickness / zoom;
        auto fill = fields->fill;
        auto fillForRect = fields->fillForRect;
        auto linesToCenter = fields->linestoCenter;
        auto hideOnNonLayer = fields->hideOnNonLayer;
        auto currentLayer = m_currentLayer;
        for (auto obj : CCArrayExt<ParticleGameObject*>(m_particleObjects)) {
            auto objCol1 = col1;
            auto objCol2 = col2;
            auto objCol3 = col3;
            if (!(currentLayer == -1 || obj->m_editorLayer == currentLayer || obj->m_editorLayer == currentLayer)) {
                if (hideOnNonLayer) continue;
                else {
                    objCol1 = ccc4f(col1.r, col1.g, col1.b, col1.a * 0.35f);
                    objCol2 = ccc4f(col2.r, col2.g, col2.b, col2.a * 0.35f);
                    objCol3 = ccc4f(col3.r, col3.g, col3.b, col3.a * 0.35f);
                }
            }
            ParticleStruct particleData;
            GameToolbox::particleStringToStruct(obj->m_particleData, particleData); // robtop stores particle shit in this string, its just particle struct but seperated with "a"s

            auto pos = obj->getPosition();
            auto maxScale = std::max(obj->m_scaleX, obj->m_scaleY);

            if (particleData.EmitterMode == 1) {
                auto startRad = particleData.StartRadius * maxScale;
                auto endRad = particleData.EndRadius * maxScale;
                auto startRadVar = particleData.StartRadiusVar * maxScale;
                auto endRadVar = particleData.EndRadiusVar * maxScale;

                if (endRad == 0.0f && endRadVar != 0.0f) drawNode->drawCircle(pos, endRadVar, ccc4f(objCol2.r, objCol2.g, objCol2.b, objCol2.a * fill), thickness, objCol2, 100); // negative thickness not a great idea
                else if (endRad != 0.0f) {
                    if (endRadVar != 0.0f) for (auto radius : {endRad + endRadVar, endRad - endRadVar}) drawNode->drawCircle(pos, radius, emptyCCC4F, thickness, objCol2, 100);
                    drawNode->drawCircle(pos, endRad, emptyCCC4F, (endRadVar != 0.0f ? endRadVar : 1.0f), ccc4f(objCol2.r, objCol2.g, objCol2.b, objCol2.a * fill), 100);
                } else drawNode->drawCircle(pos, 2.5f, objCol2, 0.0f, emptyCCC4F, 25); // hard coded thing to show when radius and var are 0

                if (startRad == 0.0f && startRadVar != 0.0f) drawNode->drawCircle(pos, startRadVar, ccc4f(objCol1.r, objCol1.g, objCol1.b, objCol1.a * fill), thickness, objCol1, 100);
                else if (startRad != 0.0f) {
                    if (startRadVar != 0.0f) for (auto radius : {startRad + startRadVar, startRad - startRadVar}) drawNode->drawCircle(pos, radius, emptyCCC4F, thickness, objCol1, 100);
                    drawNode->drawCircle(pos, startRad, emptyCCC4F, (startRadVar != 0.0f ? startRadVar : 1.0f), ccc4f(objCol1.r, objCol1.g, objCol1.b, objCol1.a * fill), 100);
                } else drawNode->drawCircle(pos, 2.5f, objCol1, 0.0f, emptyCCC4F, 25);
                
                if (linesToCenter) {
                    auto lineDistance = std::max(startRad + startRadVar, endRad + endRadVar) * 0.707106; // close enough this saves a calculation
                    for (auto pos2 : {ccp(pos.x + lineDistance, pos.y + lineDistance), ccp(pos.x - lineDistance, pos.y + lineDistance), ccp(pos.x - lineDistance, pos.y - lineDistance), ccp(pos.x + lineDistance, pos.y -lineDistance)}) drawNode->drawSegment(pos, pos2, centerLineThickness, objCol3);
                }
            }
            else {
                auto rotation = -obj->getRotation();
                auto xRange = particleData.PosVarX * obj->m_scaleX;
                auto yRange = particleData.PosVarY * obj->m_scaleY;

                CCPoint points[4] = {
                    ccp(pos.x + xRange, pos.y + yRange),
                    ccp(pos.x - xRange, pos.y + yRange),
                    ccp(pos.x - xRange, pos.y - yRange),
                    ccp(pos.x + xRange, pos.y - yRange)
                };

                if (rotation == 0);
                else { // learning trigonometry for ts smh
                    float rad = rotation * (M_PI / 180.0f);
                    auto cos = std::cos(rad);
                    auto sin = std::sin(rad);
                    for (int i = 0; i < 4; i++) {
                        auto& point = points[i];
                        auto dx = point.x - pos.x;
                        auto dy = point.y - pos.y;
                        point = ccp(pos.x + ((dx * cos) - (dy * sin)), pos.y + ((dx * sin) + (dy * cos)));
                    }
                }
                drawNode->drawPolygon(points, 4, ccc4f(objCol1.r, objCol1.g, objCol1.b, objCol1.a * (fillForRect ? fill : 0.0f)), thickness, objCol1); 
                if (linesToCenter) for (int i = 0; i < 4; i++) drawNode->drawSegment(points[i], pos, thickness, objCol3);
            }
        }
    }
};