#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

auto mod = Mod::get();

const auto emptyCCC4F = ccc4f(0.0f, 0.0f, 0.0f, 0.0f);

class $modify(LevelEditorLayer) {
    struct Fields {
        CCDrawNode* particleBoxDrawNode;
        CCLayer* batchLayer;

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
            // ive been using ts forever now and now i realise i could just use m_drawgridlayer->m_objectnode but wtv its habit atp
            if (auto shaderLayer = this->getChildByType<ShaderLayer>(0)) m_fields->batchLayer = shaderLayer->getChildByType<CCNode>(1)->getChildByType<CCLayer>(0);
            else m_fields->batchLayer = this->getChildByType<CCNode>(1)->getChildByType<CCLayer>(0);

            auto particleBoxDrawNode = CCDrawNode::create();
            particleBoxDrawNode->setPosition(ccp(0.0f, 0.0f));
            particleBoxDrawNode->setZOrder(1401);
            particleBoxDrawNode->setID("particle-box-draw-node");
            particleBoxDrawNode->setBlendFunc({GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
            m_fields->batchLayer->addChild(particleBoxDrawNode);
            m_fields->particleBoxDrawNode = particleBoxDrawNode;
        return true;
    }

    void updateDebugDraw() {
        LevelEditorLayer::updateDebugDraw();
        updateParticleRender();
    }

    void updateParticleRender() {
        auto particleObjs = ccArrayToVector<ParticleGameObject*>(m_particleObjects);
        if (particleObjs.empty()) return;
        auto fields = m_fields.self();
        if (auto drawNode = fields->particleBoxDrawNode){
            drawNode->clear();
            auto col1 = fields->col1;
            auto col2 = fields->col2;
            auto col3 = fields->col1ForCenterLines ? col1 : fields->col3;
            auto zoom = fields->batchLayer->getScale();
            auto thickness = fields->thickness / zoom;
            auto centerLineThickness = fields->centerLineThickness / zoom;
            auto fill = fields->fill;
            auto fillForRect = fields->fillForRect;
            auto linesToCenter = fields->linestoCenter;
            auto hideOnNonLayer = fields->hideOnNonLayer;
            auto currentLayer = m_currentLayer;
            for (auto obj : particleObjs) {
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
                std::vector<std::string> particleData;
                std::stringstream ss(obj->m_particleData);
                std::string str;
                while (std::getline(ss, str, 'a')) particleData.push_back(str); // robtop stores particle shit in this string, its just particle struct but seperated with "a"s

                auto pos = obj->getPosition();
                auto scale = ccp(obj->m_scaleX, obj->m_scaleY);
                auto maxScale = std::max(scale.x, scale.y);

                auto radial = std::stoi(particleData[51]) == 1;

                if (radial) {
                    auto startRad = std::stof(particleData[45]) * maxScale;
                    auto endRad = std::stof(particleData[47]) * maxScale;
                    auto startRadVar = std::stof(particleData[46]) * maxScale;
                    auto endRadVar = std::stof(particleData[48]) * maxScale;

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
                    auto xRange = std::stof(particleData[9]) * scale.x;
                    auto yRange = std::stof(particleData[10]) * scale.y;

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
    }
};