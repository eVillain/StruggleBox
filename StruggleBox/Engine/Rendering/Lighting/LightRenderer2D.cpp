#include "LightRenderer2D.h"
#include "Light2D.h"

#include "Renderer.h"
#include "Options.h"

static LightRenderer2D* g_lightRenderer = NULL;

LightRenderer2D::LightRenderer2D( Renderer* renderer ) {
    m_renderer = renderer;
    g_lightRenderer = this;
    
    renderedTris = 0;
    FBO_width = 1024;
    FBO_height = 1024;
    InitFrameBuffers();
}
LightRenderer2D::~LightRenderer2D() {
    m_renderer = NULL;
    g_lightRenderer = NULL;
    ReleaseBuffers();
}

//  - LIGHT FRAME BUFFER AND TEXTURES - //

void LightRenderer2D::InitFrameBuffers(void) {

    glGenTextures(1, &light_texture);
    glBindTexture(GL_TEXTURE_2D, light_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_width, FBO_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
//    glGenTextures(1, &glow_texture);
//    glBindTexture(GL_TEXTURE_2D, glow_texture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_width, FBO_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &light_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, light_texture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf( "[LightSys2D] Couldn't create light frame buffer\n" );
        exit(0); // Exit the application
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void LightRenderer2D::ReleaseBuffers(void) {
    // Clear lighting texture and depth framebuffers
    glDeleteFramebuffers(1, &light_fbo);
    glDeleteTextures(1, &light_texture);
//    glDeleteTextures(1, &glow_texture);
}

void LightRenderer2D::RenderLights(std::vector<Light2D *> &lights, void* space, GLuint renderFBO ) {
    if ( m_renderer == NULL ) return;
//    if ( Options::getInst()->GetBool("r_useShaders") ) return;
    GLfloat camX = 0.0f;
    GLfloat camY = 0.0f;
    GLfloat camScale = 1.0f;
    float resX = m_renderer->windowWidth;
    float resY = m_renderer->windowHeight;
    glm::vec2 camPosPX = glm::vec2( camX*camScale, camY*camScale );
    
//    glEnableClientState(GL_VERTEX_ARRAY);

    for ( unsigned int i=0; i < lights.size(); i++ ) {
        Light2D * theLight = lights.at( i );
        glm::vec2 lightPos = glm::vec2(theLight->position.x, theLight->position.y);
        float lightRadius = theLight->lightWidth;
        
        // Units in pixels
        float lightRadiusPX = lightRadius*camScale;
        if ( lightRadiusPX < 2.0f ) continue;
        glm::vec2 lightPosPX = lightPos * camScale;
        
        float lightAngle = 0.0f;
//            if ( theLight->entityBody ) { lightAngle = theLight->entityBody->getAngle(); }
        Color lightColor = theLight->lightColor;
        
        // Intersect light with screen
        Rect2D lightRectPX = Rect2D( (float)(lightPosPX.x-lightRadiusPX), (float)(lightPosPX.y-lightRadiusPX),
                                    (float)(lightRadiusPX*2.0f), (float)(lightRadiusPX*2.0f) );
        Rect2D screenRectPX = Rect2D( (float)(-camPosPX.x-resX*0.5f), (float)(-camPosPX.y-resY*0.5f),
                                     (float)resX, (float)resY );
        Rect2D lightScrnRectPX = Rect2D::GetIntersection( lightRectPX, screenRectPX );
        // Non pixel scaled rects for visualization
        Rect2D lightRect = Rect2D( (float)(lightRectPX.x/camScale), (float)(lightRectPX.y/camScale),
                                  (float)(lightRectPX.w/camScale), (float)(lightRectPX.h/camScale) );
        Rect2D screenRect = Rect2D((float)(screenRectPX.x/camScale), (float)(screenRectPX.y/camScale),
                                   (float)(screenRectPX.w/camScale), (float)(screenRectPX.h/camScale) );
        Rect2D lightScrnRect = Rect2D((float)(lightScrnRectPX.x/camScale), (float)(lightScrnRectPX.y/camScale),
                                      (float)(lightScrnRectPX.w/camScale), (float)(lightScrnRectPX.h/camScale) );
        
        // Dynamically scale the light rendering resolution
        GLfloat renderScale = 1.0f;
        glm::vec2 viewPort = glm::vec2(lightRadiusPX*2.0f, lightRadiusPX*2.0f);
        glm::vec2 lightFBOPos = lightPosPX - glm::vec2(lightScrnRectPX.x, lightScrnRectPX.y);
        
//        if ( Options::getInst()->GetBool("r_debugLights") ) {
////                Renderer::Render2D();
//            m_renderer->Draw2DRect(lightRect, COLOR_YELLOW);
//            m_renderer->Draw2DRect(screenRect, COLOR_YELLOW);
//            m_renderer->Draw2DRect(lightScrnRect, COLOR_GREEN);
//        }
        
        // Viewport scaling
        float scaleX = FBO_width/lightScrnRectPX.w;
        float scaleY = FBO_height/lightScrnRectPX.h;
        renderScale = scaleX < scaleY ? scaleX : scaleY;
        viewPort = glm::vec2( lightScrnRectPX.w*renderScale, lightScrnRectPX.h*renderScale );
        
        // FBO texture coordinates range 0.0 to 1.0
        GLfloat texCoordBottom = 0.0f;
        GLfloat texCoordLeft = 0.0f;
        GLfloat texCoordTop = (GLfloat)(viewPort.y/FBO_height);
        GLfloat texCoordRight = (GLfloat)(viewPort.x/FBO_width);
        
        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo); // Bind our frame buffer for rendering
        glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT); // Push our glEnable and glViewport states
        {
            glViewport(0, 0, (GLsizei)viewPort.x, (GLsizei)viewPort.y); // Set the size of the frame buffer view port
            
//            if ( Options::getInst()->GetBool("r_debugLights") )
//            { glClearColor(1.0f, 0.0f, 0.0f, 0.2f); }
//            else
            { glClearColor(0.0f, 0.0f, 0.0f, 0.0f); }
            
            glClear( GL_COLOR_BUFFER_BIT ); // Clear the color buffer
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);       // Disable texturing so we can draw the light gradient
            glMatrixMode(GL_PROJECTION);    // Select The Projection Matrix
            
            glPushMatrix(); {
                glLoadIdentity();
                glOrtho(0, viewPort.x, 0.0, viewPort.y, -1.0, 1.0);
                glTranslatef(0.0f, 0.0f, 0.0f);
                glScalef(renderScale, renderScale, 1.0f);
                
                int segs = (int)(lightRadiusPX/4);
                if ( segs < 4 ) segs = 4;
                if ( segs > 128 ) segs = 128;
                
                // Render light circle or beam
                //                    if (theLight->subType == LIGHT_SPOT ||
                //                        theLight->subType == LIGHT_SUN )
                { Draw2DLightCircle(lightFBOPos, (float)lightRadiusPX, (float)lightAngle, segs, lightColor); }
                //                    else if (theLight->subType == LIGHT_BEAM)
                //                    { LightRenderer::DrawLightBeam( cpv(0.0f, lightFBOPos.y), (float)(theLight->lightArc), (float)lightRadiusPX, (float)lightAngle, segs, lightColor); }
                
                // Move and scale rendering for shadows
                glScalef(camScale, camScale, 1.0f);
                glm::vec2 lightOffset = glm::vec2(lightRect.x-lightScrnRect.x, lightRect.y-lightScrnRect.y);
                //                    if (theLight->subType == LIGHT_SPOT ||
                //                        theLight->subType == LIGHT_SUN )
                { glTranslatef( (GLfloat)(-lightPos.x+lightOffset.x+lightRadius), (GLfloat)(-lightPos.y+lightOffset.y+lightRadius), 0.0f); }
                //                    else if (theLight->subType == LIGHT_BEAM)
                //                    { glTranslatef( (GLfloat)(-lightPos.x+lightOffset.x+lightRadius), (GLfloat)(-lightPos.y+lightOffset.y+lightRadius), 0.0f); }
                if ( space ) {
                    //                        if ( Options::getInst()->r_colorShadows ) {
                    ////                            theLight->subType == LIGHT_SUN ) {
                    //                            glEnable(GL_BLEND);
                    //                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Blend object color with shadow
                    //                            glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
                    //                            // Render shadows for that light
                    //                            RenderShadowsForLight(theLight, space);
                    //                            glBlendEquation(GL_FUNC_ADD);
                    //                        } else {
                    // Render hard shadows for that light
//                    RenderShadowsForLight(theLight, (cpSpace*)space);
                    //                        }
                }
                
//                if ( Options::getInst()->GetBool("r_debugLights") ) {
                    //                        glLoadIdentity();
                    //                        glOrtho(0, viewPort.x, 0.0, viewPort.y, -1.0, 1.0);
                    //                        glTranslatef(0.0f, 0.0f, 0.0f);
                    //                        Renderer::DrawCross(cpv(viewPort.x*0.5f, viewPort.y*0.5f), viewPort.x/2, 0.0f);
                    //                        Renderer::DrawCross(cpv(1.0f, 1.0f), viewPort.x/2, 0.0f);
                    //                        Renderer::DrawCross(cpv(viewPort.x, viewPort.y), viewPort.x/2, 0.0f);
//                }
            } glPopMatrix();
        } glPopAttrib(); // Restore our glEnable and glViewport states
        
        // The light_fbo now contains the rendered light and shadows
        
        glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);               // Unbind our light framebuffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);        // Screen blend light colors

        m_renderer->DrawTexture(lightScrnRect, Rect2D(texCoordLeft,texCoordBottom,texCoordRight-texCoordLeft,texCoordTop-texCoordBottom), light_texture, 10.0f);
    }
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA);
}

//static void RenderShadowForLight(cpShape*shape, void* data) {
//    LightComponent* light = (LightComponent*)data;
//    cpVect lightPos = cpv(light->position.x, light->position.y);
//    //        if (light->subType == LIGHT_BEAM) {
//    //            lightPos.x -= light->lightWidth;
//    //        }
//    //        if ( !Options::getInst()->r_entityShadows ) {
//    //            if (shape->collision_type == COLLISION_LIGHT    ||
//    //                shape->collision_type == COLLISION_WHEEL    ||
//    //                shape->collision_type == COLLISION_HEAD     ||
//    //                shape->collision_type == COLLISION_TORSO    ||
//    //                shape->collision_type == COLLISION_ARM      ||
//    //                shape->collision_type == COLLISION_LEG      ||
//    //                shape->collision_type == COLLISION_FOOT     ||
//    //                shape->collision_type == COLLISION_ITEM     ||
//    //                shape->collision_type == COLLISION_BLADE )
//    //            { return; }
//    //        }
//    bool renderInfiniteShadow = false;
//    if (shape->body == shape->space_private->staticBody) {
//        if (!cpSpacePointQueryFirst(shape->space_private, lightPos, CP_ALL_LAYERS, CP_NO_GROUP)) {
//            renderInfiniteShadow = true;
//        }
//    }
//    Color shadowColor = COLOR_NONE;
//    //        if ( Options::getInst()->r_colorShadows ) {
//    //            Shape* superShape = (Shape*)shape->data;
//    //            shadowColor.a = superShape->fillColor.a;
//    //            shadowColor.r = 1-superShape->fillColor.r;
//    //            shadowColor.g = 1-superShape->fillColor.g;
//    //            shadowColor.b = 1-superShape->fillColor.b;
//    //        }
//    //        if (light->subType == LIGHT_SUN) {
//    //            shadowColor = light->color;
//    //            // Sun shadow factor
//    //            shadowColor.a *= 0.1f;
//    //            renderInfiniteShadow = false;
//    //        }
//    g_lightRenderer->DrawShapeShadow(shape, light->lightWidth, lightPos, shadowColor, renderInfiniteShadow);
//}
//void LightRenderer2D::RenderShadowsForLight(LightComponent* light, cpSpace * space) {
//    cpSpaceEachShape(space, RenderShadowForLight, light);
//}


//  - POLYGON SHADOW PROJECTION - //
glm::vec2 LightRenderer2D::ProjectVertForLight(glm::vec2 vert, glm::vec2 light) {
    glm::vec2 lightToPoint = vert - light;
    glm::vec2 projectedPoint = vert + lightToPoint;
    return projectedPoint;
}
glm::vec2 LightRenderer2D::ProjectVertForLightToInfinity(glm::vec2 vert, glm::vec2 light) {
    glm::vec2 lightToPoint = vert - light;
    lightToPoint = glm::normalize(lightToPoint)*1000000.0f;   // yeah ok 1000000 isn't really infinity but it's close enough
    glm::vec2 projectedPoint = vert + lightToPoint;
    return projectedPoint;
}
glm::vec2 LightRenderer2D::ProjectVertForLightToRadius(glm::vec2 vert, glm::vec2 light, float radius) {
    glm::vec2 lightToPoint = vert - light;
    float lightToPointLength = glm::length(lightToPoint);
    if (lightToPointLength > radius) {
        glm::vec2 projectedPoint = vert + lightToPoint;
        return projectedPoint;
    } else {
        float extraLengthNeeded = radius - lightToPointLength;
        glm::vec2  additionVector = glm::normalize(lightToPoint) * extraLengthNeeded;
        glm::vec2 projectedPoint = vert + additionVector;
        return projectedPoint;
    }
}
//  - Test if edge casts a shadow or is facing light - //
bool LightRenderer2D::DoesEdgeCastShadow(glm::vec2 start, glm::vec2 end, glm::vec2 light) {
    glm::vec2 startToEnd = end - start;
    glm::vec2 normal = glm::vec2(startToEnd.y, -1 * startToEnd.x);
    glm::vec2 lightToStart = start - light;
    if (glm::dot(normal, lightToStart) < 0) { return true; }
    else { return false; }
}
    //  - Render shadow for verts - //
    void LightRenderer2D::RenderShadow(glm::vec2 vert1, glm::vec2 vert2, glm::vec2 lightCenter,
                                       Color shadowColor, bool infiniteShadow) {
//        float alpha = 0.0f;
        if ( DoesEdgeCastShadow(vert1, vert2, lightCenter) ) {
            glm::vec2 verts[4];
            verts[0] = vert1;
            if (infiniteShadow) {
                verts[1] = ProjectVertForLightToInfinity(vert1, lightCenter);
                verts[2] = ProjectVertForLightToInfinity(vert2, lightCenter);
            } else {
                verts[1] = ProjectVertForLight(vert1, lightCenter);
                verts[2] = ProjectVertForLight(vert2, lightCenter);
            }
            verts[3] = vert2;
            
            m_renderer->DrawPolygon(4, verts, COLOR_NONE, shadowColor);

////            Color shadowColor = {0.0f, 0.0f, 0.0f, alpha};
//            glVertexPointer(2, GL_FLOAT, 0, verts);
//
//            glColorFromColor(shadowColor);
//            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
////            Renderer::SetRenderedTris( Renderer::GetRenderedTris()+4 );
//            renderedTris += 4;
            
//            if ( Options::getInst()->GetBool("r_debugLights") ) {
//                m_renderer->Buffer2DLine(lightCenter, vert1, shadowColor, shadowColor);
//                m_renderer->Buffer2DLine(lightCenter, vert2, shadowColor, shadowColor);
//                m_renderer->Buffer2DLine(vert1, vert2, shadowColor, shadowColor);
//                m_renderer->DrawPolygon(4, verts, shadowColor, LAColor(0.0, 0.0), 1.0f);
//            }
        }
    }
void LightRenderer2D::RenderCircleShadow(glm::vec2 center1, glm::vec2 center2,
                                         float angle, float radius1, float radius2, float range, Color shadowColor) {
    int shadowSegs = (int)(9*range);   // dynamic LOD, fuck yeah :p
    if ( shadowSegs > 64 ) shadowSegs = 64;
    //range = 1.0f; // 1 is half circle, 2 is full circle
    const float coef1 = (float)(range*M_PI/shadowSegs); // part of circle
    const float coef2 = (float)(M_PI/shadowSegs); // half circle
    
    GLfloat* vertices = new GLfloat[2*(2*shadowSegs+2)];
    GLfloat angle2 =  (GLfloat)(angle + M_PI_2 - (M_PI_2*range));
    
    if( ! vertices )
        return;
    
    for(int i = 0;i <= shadowSegs; i++) {
        float rads = (i)*coef1;
        GLfloat j = (GLfloat)(radius1 * cosf(rads + angle2) + center1.x);
        GLfloat k = (GLfloat)(radius1 * sinf(rads + angle2) + center1.y);
        vertices[(i)*4] = j;
        vertices[(i)*4+1] = k;
        
        rads = (i)*coef2;
        GLfloat l = (GLfloat)(radius2 * cosf(rads + (float)angle) + center2.x);
        GLfloat m = (GLfloat)(radius2 * sinf(rads + (float)angle) + center2.y);
        vertices[(i)*4+2] = l;
        vertices[(i)*4+3] = m;
    }
    
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glPushMatrix(); {
        glColor4fv((GLfloat *)&shadowColor);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*shadowSegs+2);
        //            Renderer::SetRenderedTris( Renderer::GetRenderedTris()+2*shadowSegs+2 );
        renderedTris += 2*shadowSegs+2;
    } glPopMatrix();
    delete[] vertices;
}
//void LightRenderer2D::DrawShapeShadow(cpShape *shape, cpFloat lightRadius, cpVect lightCenter, Color shadowColor, bool infiniteShadow) {
//    cpBody * body = shape->body;
//    bool skipSegment = false;
//    
//    if (shape->CP_PRIVATE(klass)->type == CP_POLY_SHAPE) {
//        int vertCount = cpPolyShapeGetNumVerts(shape);
//        for (int i = 0; i < vertCount; i++) {
//            skipSegment = false;
//            cpVect vert1 = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, i));
//            cpVect vert2;
//            if (i != vertCount -1) { vert2 = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, i+1)); }
//            else { vert2 = cpBodyLocal2World(body, cpPolyShapeGetVert(shape, 0)); }
//            cpVect centerToV1 = cpvsub(vert1, lightCenter);
//            cpVect centerToV2 = cpvsub(vert2, lightCenter);
//            cpFloat radiusSq = lightRadius*lightRadius;
//            if (cpvlengthsq(centerToV1) > radiusSq && cpvlengthsq(centerToV2) > radiusSq) { skipSegment = cpTrue; }
//            if (skipSegment == false) { RenderShadow(vert1, vert2, lightCenter, shadowColor, infiniteShadow); }
//        }
//    } else if (shape->CP_PRIVATE(klass)->type == CP_CIRCLE_SHAPE) {
//        skipSegment = false;
//        cpCircleShape *circle = (cpCircleShape *)shape;
//        cpVect circleCenter = circle->tc;
//        cpFloat circleRadius = circle->r;
//        cpVect lightToCenter = cpvsub(circleCenter, lightCenter);
//        
//        cpFloat distance = cpvdist(circleCenter, lightCenter)-circleRadius;
//        cpFloat distanceRatio = distance/circleRadius;
//        if (distanceRatio > 2.0) distanceRatio = 2.0f;
//        cpVect normal = cpvnormalize(cpv(lightToCenter.y, -1 * lightToCenter.x));
//        cpFloat shadowAngle = cpvtoangle(normal); // in radians
//        normal = cpvmult(normal, circleRadius);
//        cpVect vert1 = cpvsub(circleCenter, normal);
//        cpVect vert2 = cpvadd(circleCenter, normal);
//        float shadowRadius = (float)(cpvlength(cpvsub(vert1, vert2))/distanceRatio);
//        
//        if (infiniteShadow) {
//            // Yeah ok, lightRadius isn't really infinity but it's close enough
//            lightToCenter = cpvmult(cpvnormalize(lightToCenter), lightRadius);
//        }
//        if (distance < (circleRadius)) {
//            RenderCircleShadow(circleCenter, cpvadd(circleCenter, lightToCenter), shadowAngle, circleRadius, shadowRadius, 2.0-distanceRatio, shadowColor);
//        } else {
//            RenderCircleShadow(circleCenter, cpvadd(circleCenter, lightToCenter), shadowAngle, circleRadius, shadowRadius, 1.0f, shadowColor);
//        }
//    }
//    else if (shape->CP_PRIVATE(klass)->type == CP_SEGMENT_SHAPE)
//    {
//        cpSegmentShape *seg = (cpSegmentShape *)shape;
//        cpVect vert1 = cpBodyLocal2World(body, seg->a);
//        cpVect vert2 = cpBodyLocal2World(body, seg->b);
//        cpVect centerToV1 = cpvsub(vert1, lightCenter);
//        cpVect centerToV2 = cpvsub(vert2, lightCenter);
//        cpFloat radiusSq = lightRadius*lightRadius*lightRadius;
//        if (cpvlengthsq(centerToV1) > radiusSq && cpvlengthsq(centerToV2) > radiusSq) { skipSegment = true; }
//        if (!skipSegment) {
//            if ( DoesEdgeCastShadow(vert1, vert2, lightCenter) ) {
//                cpVect verts[4];
//                verts[0] = vert1;
//                if (infiniteShadow || shape->body->m == INFINITY) {
//                    verts[1] = ProjectVertForLightToInfinity(vert1, lightCenter);
//                    verts[2] = ProjectVertForLightToInfinity(vert2, lightCenter);
//                }
//                else {
//                    verts[1] = ProjectVertForLight(vert1, lightCenter);
//                    verts[2] = ProjectVertForLight(vert2, lightCenter);
//                }
//                verts[3] = vert2;
//                
//                Color shadowColor = {0.0f, 0.0f, 0.0f, 0.0f};
//#if CP_USE_DOUBLES
//                glVertexPointer(2, GL_DOUBLE, 0, verts);
//#else
//                glVertexPointer(2, GL_FLOAT, 0, verts);
//#endif
//                glColorFromColor(shadowColor);
//                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//                //                    Renderer::SetRenderedTris( Renderer::GetRenderedTris()+4 );
//                renderedTris += 4;
//            }
//            else if ( DoesEdgeCastShadow(vert2, vert1, lightCenter) ) {
//                cpVect verts[4];
//                verts[0] = vert2;
//                if (infiniteShadow || shape->body->m == INFINITY) {
//                    verts[1] = ProjectVertForLightToInfinity(vert2, lightCenter);
//                    verts[2] = ProjectVertForLightToInfinity(vert1, lightCenter);
//                }
//                else {
//                    verts[1] = ProjectVertForLight(vert2, lightCenter);
//                    verts[2] = ProjectVertForLight(vert1, lightCenter);
//                }
//                verts[3] = vert1;
//                Color shadowColor = {0.0f, 0.0f, 0.0f, 0.0f};
//#if CP_USE_DOUBLES
//                glVertexPointer(2, GL_DOUBLE, 0, verts);
//#else
//                glVertexPointer(2, GL_FLOAT, 0, verts);
//#endif
//                glColorFromColor(shadowColor);
//                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//                //                    Renderer::SetRenderedTris( Renderer::GetRenderedTris()+4 );
//                renderedTris += 4;
//            }
//        }
//    }
//}
void LightRenderer2D::Draw2DLightCircle( glm::vec2 center, float radius, float angle, int segs, Color lightColor) {
    const float coef = 2.0f * (float)M_PI/segs;
    
    GLfloat* vertices = new GLfloat[2*(segs+2)];
    
    if( ! vertices )
        return;
    Color* colors = new Color[2*(segs+2)];
    if( ! colors )
        return;
    Color outerColor = {0.0f, 0.0f, 0.0f, 0.0f};
    
    for(int i = 0;i <= segs; i++) {
        float rads = i*coef;
        GLfloat j = radius * cosf(rads + angle);
        GLfloat k = radius * sinf(rads + angle);
        
        vertices[(i+1)*2] = j;
        vertices[(i+1)*2+1] = k;
        
        colors[i*2] =  outerColor;
        colors[i*2+1] =  outerColor;
    }
    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    colors[0] = lightColor;
    
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, colors);
    glEnableClientState(GL_COLOR_ARRAY);
    glPushMatrix(); {
        glTranslatef((GLfloat)center.x, (GLfloat)center.y, 0.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segs + 2);
//        m_renderer::SetRenderedTris( Renderer::GetRenderedTris()+segs+2 );
        renderedTris += segs + 2;
    } glPopMatrix();
    glDisableClientState(GL_COLOR_ARRAY);
    delete[] vertices;
    delete[] colors;
}
void LightRenderer2D::Draw2DLightBeam( glm::vec2 center, float arc, float radius, float angle, int segs, Color lightColor) {
    const float coef = (float)arc/segs;
    const float halfArc = arc*0.5f;
    
    GLfloat* vertices = new GLfloat[2*(segs+2)];
    
    if( ! vertices )
        return;
    Color* colors = new Color[2*(segs+2)];
    if( ! colors )
        return;
    Color outerColor = {0.0f, 0.0f, 0.0f, 0.0f};
    
    for(int i = 0;i <= segs; i++) {
        float rads = i*coef;
        GLfloat j = radius * cosf(rads + angle - halfArc);
        GLfloat k = radius * sinf(rads + angle - halfArc);
        
        vertices[(i+1)*2] = j;
        vertices[(i+1)*2+1] = k;
        
        colors[i*2] =  outerColor;
        colors[i*2+1] =  outerColor;
    }
    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    colors[0] = lightColor;
//    if ( Options::getInst()->GetBool("r_useShaders") ) {
//        
//    } else {
        glVertexPointer(2, GL_FLOAT, 0, vertices);
        glColorPointer(4, GL_FLOAT, 0, colors);
        glEnableClientState(GL_COLOR_ARRAY);
        glPushMatrix(); {
            glTranslatef((GLfloat)center.x, (GLfloat)center.y, 0.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, segs + 2);
            renderedTris += segs + 2;
        } glPopMatrix();
        glDisableClientState(GL_COLOR_ARRAY);
//    }
}
//void LightRenderer2D::DrawShapeGlow(cpShape *shape) {
//    cpFloat zVal = 0.0f;
//    cpBody *body = shape->body;
//    Color color = COLOR_STATIC_BRIGHT;
//    
//    switch(shape->CP_PRIVATE(klass)->type){
//        case CP_CIRCLE_SHAPE: {
//            //                cpCircleShape *circle = (cpCircleShape *)shape;
//            //                if (shape->collision_type == COLLISION_PLAYER_SS ||
//            //                    shape->collision_type == COLLISION_PLAYER_TD) {
//            //                    color = COLOR_PLAYER;
//            //                }
//            //                else if (shape->collision_type == COLLISION_AGENT) {
//            //                    color = COLOR_AGENT;
//            //                }
//            //                else if (shape->collision_type == COLLISION_PROJECTILE) {
//            //                    color = COLOR_BULLET;
//            //                }
//            //                else if (shape->collision_type == COLLISION_LIGHT) return;
//            //                Renderer::DrawCircle(circle->tc, body->a, circle->r, COLOR_NONE, color, zVal);
//            break;
//        }
//        case CP_SEGMENT_SHAPE: {
//            //                cpSegmentShape *seg = (cpSegmentShape *)shape;
//            //                Renderer::DrawFatSegment(seg->ta, seg->tb, seg->r, COLOR_NONE, color);
//            break;
//        }
//        case CP_POLY_SHAPE: {
//            //                cpPolyShape *poly = (cpPolyShape *)shape;
//            //                    if (shape->collision_type == COLLISION_CAR) {
//            //                    color = COLOR_GREY_LIGHT_VIBRANT;
//            //                }
//            //                else if (shape->collision_type == COLLISION_WHEEL) {
//            //                    color = COLOR_GREY_DARK_VIBRANT;
//            //                }
//            //                else if (shape->collision_type == COLLISION_ITEM) {
//            /*
//             else if (shape->collision_type == COLLISION_WEAPON_BAT) {
//             color = COLOR_GREY_BRIGHT_VIBRANT;
//             }
//             else if (shape->collision_type == COLLISION_WEAPON_9MM ||
//             shape->collision_type == COLLISION_WEAPON_UZI) {
//             color = COLOR_GREY_LIGHT_VIBRANT;
//             }
//             else if (shape->collision_type == COLLISION_WEAPON_AK47) {
//             color = COLOR_GREY_MEDIUM_VIBRANT;
//             }
//             */
//            //                    color = COLOR_GREY_BRIGHT_VIBRANT;
//            //                }
//            //                Renderer::DrawPolygon(poly->numVerts, poly->tVerts, COLOR_NONE, color, zVal);
//            break;
//        }
//        default: break;
//    }
//}
void LightRenderer2D::BlurTextureRadial (unsigned int tex, int passes, float texWidth, float texHeight, float renderWidth, float renderHeight)
{
    int i;
    float wRatio = renderWidth/texWidth;
    float hRatio = renderHeight/texHeight;
    
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture (GL_TEXTURE_2D, tex);
    GLfloat verts[] = {
        0,  0,
        0,  renderHeight,
        renderWidth, renderHeight,
        renderWidth, 0
    };
    GLfloat texCoords[] = {
        0, 0,
        0, hRatio,
        wRatio, hRatio,
        wRatio, 0
    };
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    
    while (passes > 0) {
        for (i = 0; i < 2; i++) {
            glPushMatrix (); {
                //glLoadIdentity ();
                if (i == 1) {
                    glTranslatef (texWidth/2, texHeight/2,0);
                    glRotatef (1, 0, 0, 1);
                    glTranslatef (-texWidth/2, -texHeight/2,0);
                }
                glColor4f (1.0f,1.0f,1.0f,1.0f / (i+1));
                glDrawArrays(GL_QUADS, 0, 4);
                //                    Renderer::SetRenderedTris( Renderer::GetRenderedTris()+4 );
                renderedTris += 4;
            } glPopMatrix ();
        }
        glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, (GLsizei)texWidth, (GLsizei)texHeight, 0);
        passes--;
    }
    glDisable (GL_BLEND);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
void LightRenderer2D::BlurTextureZoom (unsigned int tex, int passes, float texWidth, float texHeight, float renderWidth, float renderHeight)
{
    int i;
    float wRatio = renderWidth/texWidth;
    float hRatio = renderHeight/texHeight;
    
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture (GL_TEXTURE_2D, tex);
    while (passes > 0) {
        for (i = 0; i < 2; i++) {
            glColor4f (1.0f,1.0f,1.0f,1.0f / (i+1));
            glBegin (GL_QUADS);
            glTexCoord2f ( (GLfloat)(0.0f - (i*0.5f)/texWidth), (GLfloat)(0.0f + (i*0.5f)/texHeight) );
            glVertex2f (0.0f, 0.0f);
            glTexCoord2f ( (GLfloat)(0.0f - (i*0.5f)/texWidth), (GLfloat)(hRatio - (i*0.5f)/texHeight) );
            glVertex2f (0.0f, renderHeight);
            glTexCoord2f ( (GLfloat)(wRatio + (i*0.5f)/texWidth), (GLfloat)(hRatio - (i*0.5f)/texHeight) );
            glVertex2f (renderWidth, renderHeight);
            glTexCoord2f ( (GLfloat)(wRatio + (i*0.5f)/texWidth), (GLfloat)(0.0f + (i*0.5f)/texHeight) );
            glVertex2f (renderWidth, 0.0f);
            glEnd ();
        }
        glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, (GLsizei)texWidth, (GLsizei)texHeight, 0);
        passes--;
    }
    glDisable (GL_BLEND);
}
void LightRenderer2D::BlurTexture (unsigned int tex, int passes, float texWidth, float texHeight, float renderWidth, float renderHeight) {
    int i, x, y;
    
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture (GL_TEXTURE_2D, tex);
    GLdouble verts[2*4];
    GLdouble texCoords[2*4];
    
    while (passes > 0) {
        i = 0;
        for (x = 0; x < 2; x++)
        {
            for (y = 0; y < 2; y++, i++)
            {
                verts[0] = 0.0f; verts[1] = 0.0f;
                verts[2] = 0.0f; verts[3] = renderHeight;
                verts[4] = renderWidth; verts[5] = 0.0f;
                verts[6] = renderWidth; verts[7] = renderHeight;
                
                texCoords[0] = 0 + (x-0.5)/texWidth; texCoords[1] = 1 + (y-0.5)/texHeight;
                texCoords[2] = 0 + (x-0.5)/texWidth; texCoords[3] = 0 + (y-0.5)/texHeight;
                texCoords[4] = 1 + (x-0.5)/texWidth; texCoords[5] = 1 + (y-0.5)/texHeight;
                texCoords[6] = 1 + (x-0.5)/texWidth; texCoords[7] = 0 + (y-0.5)/texHeight;
                
                glVertexPointer(2, GL_DOUBLE, 0, verts);
                glTexCoordPointer(2, GL_DOUBLE, 0, texCoords);
                glColor4f (1.0f,1.0f,1.0f,1.0f / (i+1));
                
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                //                    Renderer::SetRenderedTris( Renderer::GetRenderedTris()+4 );
                renderedTris += 4;
            }
        }
        glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, (GLsizei)renderWidth, (GLsizei)renderHeight, 0);
        passes--;
    }
    glDisable (GL_BLEND);
}
void LightRenderer2D::UpdateLightStats( void ) {
    //        StatTracker::SetRNumLightTris( renderedTris );
    renderedTris = 0;
}

//void LightRenderer2D::RenderGlow(cpSpace * space) {
//    float glowScale = 1/4.0f;
//    int blurPasses = 4;
//    
//    // how many pixels to render the glow into
//    GLfloat renderWidth = (GLfloat)(Options::getInst()->GetInt("r_resolutionX")*glowScale);
//    GLfloat renderHeight = (GLfloat)(Options::getInst()->GetInt("r_resolutionY")*glowScale);
//    GLfloat scaledWidth = (GLfloat)(renderWidth/FBO_width);
//    GLfloat scaledHeight = (GLfloat)(renderHeight/FBO_height);
//    
//    //first we get the glowing shapes drawn into the light_fbo
//    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, light_fbo); // Bind our frame buffer for rendering
//    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT); // Push our glEnable and glViewport states
//    {
//        glViewport(0, 0, (GLsizei)renderWidth, (GLsizei)renderHeight); // Set the size of the frame buffer view port
//        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//        //glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // red for debug
//        
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the depth and colour buffers
//        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//        glDepthMask(GL_FALSE);
//        glDisable(GL_BLEND);
//        glDisable(GL_TEXTURE_2D); // Disable texturing so we can draw the glowing objects
//        
//        glPushMatrix(); {
//            //                Renderer::Render2D();
//            
//            //render entities
//            //                for (int i=0; i < EntityList::getEntityList()->size(); i++) {
//            //                    Entity * theEntity = EntityList::getEntityList()->at(i);
//            //                    if (theEntity->getLinkedVehicle() ||
//            //                        !theEntity->isPhysicsEnabled()) continue;
//            //                    else LightRenderer::DrawShapeGlow(theEntity->entityShape->getShape());
//            //                }
//        } glPopMatrix();
//    } glPopAttrib(); // Restore our glEnable and glViewport states
//    
//    // the scaled glowing objects are in the light_fbo now
//    if (blurPasses != 0) {
//        // they should now get blurred
//        // first pass the light_fbo texture into the glow_texture for blurring
//        glBindTexture(GL_TEXTURE_2D, glow_texture);
//        glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, FBO_width, FBO_height, 0);
//        glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT); // Push our glEnable and glViewport states
//        {
//            glViewport(0, 0, FBO_width-1, FBO_height-1); // Set the size of the frame buffer view port
//            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//            //glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // red for debug
//            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the depth and colour buffers
//            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//            glDepthMask(GL_FALSE);
//            glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
//            
//            glPushMatrix(); {
//                glLoadIdentity();
//                glOrtho(0, FBO_width, 0, FBO_height, -1.0, 1.0);
//                
//                // blur our glow texture
//                BlurTexture(glow_texture, blurPasses, (float)FBO_width, (float)FBO_height, (float)renderWidth, (float)renderHeight);
//                // here's a zoom blur just for fun
//                //BlurTextureZoom(glow_texture, blurPasses, FBO_width, FBO_height, renderWidth, renderHeight);
//                // and a radial blur
//                //BlurTextureRadial(glow_texture, blurPasses, FBO_width, FBO_height, renderWidth, renderHeight);
//            }    glPopMatrix();
//        } glPopAttrib(); // Restore our glEnable and glViewport states
//        
//        // bind the blurred texture
//        //    glBindTexture(GL_TEXTURE_2D, glow_texture);
//        glBindTexture(GL_TEXTURE_2D, light_texture);
//    }
//    // render FBO to screen
//    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our fbo
//    glDepthMask(GL_TRUE);
//    glEnable(GL_BLEND);
//    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);          // additive blend glow
//    //glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);    //screen blend glow
//    glBlendFunc(GL_ONE, GL_ONE);
//    glEnable(GL_TEXTURE_2D);
//    
//    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//    GLfloat right = (GLfloat)(Options::getInst()->GetInt("r_resolutionX")/2.0);
//    GLfloat left = (GLfloat)-(Options::getInst()->GetInt("r_resolutionX")/2.0);
//    GLfloat top = (GLfloat)(Options::getInst()->GetInt("r_resolutionY")/2.0);
//    GLfloat bottom = (GLfloat)-(Options::getInst()->GetInt("r_resolutionY")/2.0);
//    
//    GLfloat verts[] = {
//        left, bottom,
//        left, top,
//        right, top,
//        right, bottom
//    };
//    GLfloat texCoords[] = {
//        0.0f, 0.0f,
//        0.0f, scaledHeight,
//        scaledWidth, scaledHeight,
//        scaledWidth, 0.0f
//    };
//    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//    glVertexPointer(2, GL_FLOAT, 0, verts);
//    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
//    
//    glPushMatrix(); {
//        // Full-screen render
//        glDrawArrays(GL_QUADS, 0, 4);
//        //            Renderer::SetRenderedTris( Renderer::GetRenderedTris()+4 );
//    } glPopMatrix();
//    // Bind on-screen texture again
//    glBindTexture(GL_TEXTURE_2D, 0);
//    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//}

