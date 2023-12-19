/*
    James William Fletcher  ( notabug.org/Vandarin )
        November 2023       ( github.com/mrbid     )

    Save your garden friends from the ghosts!
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WEB
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
#endif

#define uint GLushort
#define sint GLshort

#include "inc/gl.h"
#define GLFW_INCLUDE_NONE
#include "inc/glfw3.h"

#include "inc/esAux4.h"

#include "inc/res.h"
#include "assets/intro.h"
#include "assets/terrain.h"
#include "assets/club.h"
#include "assets/ghost.h"
#include "assets/octo.h"
#include "assets/baby.h"
#include "assets/bones.h"
#include "assets/bunny.h"
#include "assets/cat.h"
#include "assets/ginger.h"
#include "assets/farmer.h"
#include "assets/foxy.h"
#include "assets/frog.h"
#include "assets/rat.h"
#include "assets/shroom.h"
#include "assets/robot.h"
#include "assets/teddy.h"
#include "assets/treeman.h"
#include "assets/dragon.h"
#include "assets/banana.h"

#ifdef QUALITY_MEDIUM
    #include "assets/scene_d1.h"
#elif QUALITY_LOW
    #include "assets/scene_d2.h"
#else
    #include "assets/scene.h"
#endif

//*************************************
// globals
//*************************************
const char appTitle[]="☃️ AI Generated Game ❄️ (Meshy.ai) 🤖";
GLFWwindow* window;
uint winw=1024, winh=768;
float t=0.f, dt=0.f, lt=0.f, ltut=0.f, st=0.f, fc=0.f, lfct=0.f, aspect;
double mx,my,lx,ly,ww,wh;

// render state id's
GLint projection_id;
GLint modelview_id;
GLint position_id;
GLint normal_id;
GLint color_id;
GLint lightpos_id;
GLint opacity_id;

// render state
mat projection, view, model, modelview;

// models
ESModel mdlIntro;
ESModel mdlTerrain;
ESModel mdlScene; // duplicating mesh into a scene like this makes the file bigger at a disproportionate ratio to making the renderer more efficient, but it was just faster to do this.
ESModel mdlClub;
ESModel mdlGhost;
ESModel mdlOcto;
ESModel mdlBaby;
ESModel mdlBones;
ESModel mdlBunny;
ESModel mdlCat;
ESModel mdlGinger;
ESModel mdlFarmer;
ESModel mdlFoxy;
ESModel mdlFrog;
ESModel mdlRat;
ESModel mdlShroom;
ESModel mdlRobot;
ESModel mdlTeddy;
ESModel mdlTreeman;
ESModel mdlDragon;
ESModel mdlBanana;

// positions
const vec baby_pos    = (vec){  0.31471309065818787, -0.18127848207950592, -1.588475424796343e-05  };
const vec bones_pos   = (vec){  0.9441254138946533 , -4.923678398132324  , -1.8969178199768066e-05 };
const vec bunny_pos   = (vec){ -0.1463201642036438 , -3.3715295791625977 , -1.341104507446289e-06  };
const vec cat_pos     = (vec){  2.409440517425537  , -0.08113411068916321,  0.0006199628114700317  };
const vec ginger_pos  = (vec){ -3.5621275901794434 ,  1.3744210004806519 , -4.3662264943122864e-05 };
const vec farmer_pos  = (vec){ -5.638768672943115  , -2.3814315795898438 , -5.668585072271526e-05  };
const vec foxy_pos    = (vec){  0.3325018882751465 ,  3.407064437866211  , -2.3096799850463867e-06 };
const vec frog_pos    = (vec){  2.815049886703491  , -2.359147787094116  , -0.0002002716064453125  };
const vec rat_pos     = (vec){ -1.9573389291763306 , -4.990877628326416  , -5.9857964515686035e-05 };
const vec shroom_pos  = (vec){  2.9257962703704834 ,  3.9158010482788086 ,  6.631016731262207e-06  };
const vec robot_pos   = (vec){ -3.2104883193969727 ,  0                  , -2.0265579223632812e-06 };
const vec teddy_pos   = (vec){ -1.148323655128479  , -1.155029535293579  , -8.046627044677734e-06  };
const vec treeman_pos = (vec){  3.7293591499328613 ,  0.10551458597183228, -2.2292137145996094e-05 };
const vec dragon_pos  = (vec){ -5.303407669067383  , -2.8256068229675293 , -9.742379188537598e-05  };
const vec banana_pos  = (vec){  3.863978624343872  , -3.625526189804077  , -2.092123031616211e-05  };
vec friend_pos[15];

// game vars
#define FAR_DISTANCE 128.f
#define NEWGAME_SEED 1337
char tts[32];
uint keystate[5] = {0};

// camera vars
uint focus_cursor = 0;
double sens = 0.003f;
float xrot = 0.f;
float yrot = d2PI;
vec look_dir, lookx, looky;

// player vars
float MOVE_SPEED = 1.6f;
vec pp;
uint bonking = 0.f;

// ghosts
#define MAX_GHOSTS 32
vec   ghost_pos[MAX_GHOSTS];
vec   ghost_dir[MAX_GHOSTS];
int   ghost_tgt[MAX_GHOSTS];
float ghost_opa[MAX_GHOSTS];

// octo
vec octo_pos;
vec octo_dir;
int octo_tgt;

// stats
uint ghostsbonked = 0;
uint friendsalive = 15;

//*************************************
// utility functions
//*************************************
void timestamp(char* ts)
{
    const time_t tt = time(0);
    strftime(ts, 16, "%H:%M:%S", localtime(&tt));
}
void timeTaken(uint ss)
{
    if(ss == 1)
    {
        const double tt = t-st;
        if(tt < 60.0)
            sprintf(tts, "%.0f Sec", tt);
        else if(tt < 3600.0)
            sprintf(tts, "%.2f Min", tt * 0.016666667);
        else if(tt < 216000.0)
            sprintf(tts, "%.2f Hr", tt * 0.000277778);
        else if(tt < 12960000.0)
            sprintf(tts, "%.2f Days", tt * 0.00000463);
    }
    else
    {
        const double tt = t-st;
        if(tt < 60.0)
            sprintf(tts, "%.0f Seconds", tt);
        else if(tt < 3600.0)
            sprintf(tts, "%.2f Minutes", tt * 0.016666667);
        else if(tt < 216000.0)
            sprintf(tts, "%.2f Hours", tt * 0.000277778);
        else if(tt < 12960000.0)
            sprintf(tts, "%.2f Days", tt * 0.00000463);
    }
}

//*************************************
// render functions
//*************************************
void modelBind(const ESModel* mdl)
{
    glBindBuffer(GL_ARRAY_BUFFER, mdl->vid);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(position_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->iid);
}
void modelBind3(const ESModel* mdl)
{
    glBindBuffer(GL_ARRAY_BUFFER, mdl->cid);
    glVertexAttribPointer(color_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(color_id);

    glBindBuffer(GL_ARRAY_BUFFER, mdl->vid);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(position_id);

    glBindBuffer(GL_ARRAY_BUFFER, mdl->nid);
    glVertexAttribPointer(normal_id, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normal_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->iid);
}

//*************************************
// game functions
//*************************************
void retargetGhost(uint i)
{
    ghost_tgt[i] = esRand(0, 14);
    if(friend_pos[ghost_tgt[i]].z <= -0.74f)
    {
        uint fail = 1;
        for(uint j = 0; j < 15; j++)
        {
            if(friend_pos[j].z > -0.74f)
            {
                ghost_tgt[i] = j;
                fail = 0;
                break;
            }
        }
        if(fail == 1)
        {
            ghost_tgt[i] = -1;
            return;
        }
    }
    ghost_dir[i] = friend_pos[ghost_tgt[i]];
    vSub(&ghost_dir[i], ghost_dir[i], ghost_pos[i]);
    ghost_dir[i].z = 0.f;
    vNorm(&ghost_dir[i]);
}
void resetGhost(uint i)
{
    const float radius = esRandFloat(20.f, 42.f);
    const float angle = esRandFloat(-PI, PI);
    ghost_pos[i] = (vec){radius*cosf(angle), radius*sinf(angle), -1.f};
    retargetGhost(i);
    ghost_opa[i] = 1.f;
}
void newGame(unsigned int seed)
{
    srand(seed);

    xrot = 0.f;
    yrot = d2PI;
    pp = (vec){0.f, 1.f, 0.f};
    octo_pos = (vec){3.f, 1.f, 0.f};
    octo_dir = (vec){0.f, -1.f, 0.f};
    octo_tgt = -1;
    lt = 0.f;
    ltut = 0.f;

    for(uint i = 0; i < 15; i++)
        friend_pos[i].z = 0.f;

    for(uint i = 0; i < MAX_GHOSTS; i++)
        resetGhost(i);

    ghostsbonked = 0;
    friendsalive = 15;

    char strts[16];
    timestamp(&strts[0]);
    printf("[%s] 👋 Game Start 🎬 [%u] 🏁🫧🌎🥑🦄🎉.\n", strts, seed);
    
    glfwSetWindowTitle(window, appTitle);
    glfwSetTime(0.0);
}
void updateTitle()
{
    if(t < 8.f){return;}
    friendsalive = 0;
    for(uint i = 0; i < 15; i++)
    {
        if(friend_pos[i].z > -0.369f){friendsalive++;}
    }
    char tmp[256];
    sprintf(tmp, "🫂 %u - 👻 %u", friendsalive, ghostsbonked);
    glfwSetWindowTitle(window, tmp);
}

//*************************************
// update & render
//*************************************
void main_loop()
{
//*************************************
// core logic
//*************************************
    fc++;
    glfwPollEvents();
    t = (float)glfwGetTime();
    dt = t-lt;
    lt = t;

#ifdef WEB
    EmscriptenPointerlockChangeEvent e;
    if(emscripten_get_pointerlock_status(&e) == EMSCRIPTEN_RESULT_SUCCESS)
    {
        if(focus_cursor == 0 && e.isActive == 1)
        {
            glfwGetCursorPos(window, &lx, &ly);
        }
        focus_cursor = e.isActive;
    }
#endif

//*************************************
// game logic
//*************************************

    // update title every second
    if(t > ltut)
    {
        updateTitle();
        ltut = t + 1.0f;
    }

    // forward & backward
    if(keystate[2] == 1)
    {
        vec m;
        vMulS(&m, look_dir, MOVE_SPEED * dt);
        vSub(&pp, pp, m);
    }
    else if(keystate[3] == 1)
    {
        vec m;
        vMulS(&m, look_dir, MOVE_SPEED * dt);
        vAdd(&pp, pp, m);
    }

    // strafe left & right
    if(keystate[0] == 1)
    {
        mGetViewX(&lookx, view);
        vec m;
        vMulS(&m, lookx, MOVE_SPEED * dt);
        vSub(&pp, pp, m);
    }
    else if(keystate[1] == 1)
    {
        mGetViewX(&lookx, view);
        vec m;
        vMulS(&m, lookx, MOVE_SPEED * dt);
        vAdd(&pp, pp, m);
    }

    // sprint
    if(keystate[4] == 1)
    {
        MOVE_SPEED = 4.20f;
    }
    else
    {
        MOVE_SPEED = 1.60f;
    }

    // lock to floor
    pp.z = -0.26f;

//*************************************
// camera
//*************************************
    if(focus_cursor == 1)
    {
        glfwGetCursorPos(window, &mx, &my);
        xrot += (lx-mx)*sens;
        yrot += (ly-my)*sens;

        if(yrot > 2.5f)
            yrot = 2.5f;
        if(yrot < 0.55f)
            yrot = 0.55f;

        lx = mx;
        ly = my;
    }
    mIdent(&view);
    mRotate(&view, yrot, 1.f, 0.f, 0.f);
    mRotate(&view, xrot, 0.f, 0.f, 1.f);
    mTranslate(&view, pp.x, pp.y, pp.z);

    // get look dir/axes
    mGetViewZ(&look_dir, view);
    mGetViewX(&lookx, view);
    mGetViewY(&looky, view);

//*************************************
// render
//*************************************

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ///

    // shader program change
    shadeFullbright(&position_id, &projection_id, &modelview_id, &color_id, &opacity_id);
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);

    // render terrain
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&view.m[0][0]);
    modelBind(&mdlTerrain);
    glUniform3f(color_id, 1.f, 1.f, 1.f);
    glDrawElements(GL_TRIANGLES, terrain_numind, GL_UNSIGNED_BYTE, 0);

    ///

    // shader program change
    shadeLambert3(&position_id, &projection_id, &modelview_id, &lightpos_id, &normal_id, &color_id, &opacity_id);
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);

    // render scene
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&view.m[0][0]);
    modelBind3(&mdlScene);
    glDrawElements(GL_TRIANGLES, scene_numind, GL_UNSIGNED_INT, 0);

    // baby
    mIdent(&model);
    mSetPos(&model, friend_pos[0]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlBaby);
    glDrawElements(GL_TRIANGLES, baby_numind, GL_UNSIGNED_SHORT, 0);

    // bones
    mIdent(&model);
    mSetPos(&model, friend_pos[1]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlBones);
    glDrawElements(GL_TRIANGLES, bones_numind, GL_UNSIGNED_SHORT, 0);

    // bunny
    mIdent(&model);
    mSetPos(&model, friend_pos[2]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlBunny);
    glDrawElements(GL_TRIANGLES, bunny_numind, GL_UNSIGNED_SHORT, 0);

    // cat
    mIdent(&model);
    mSetPos(&model, friend_pos[3]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlCat);
    glDrawElements(GL_TRIANGLES, cat_numind, GL_UNSIGNED_SHORT, 0);

    // ginger
    mIdent(&model);
    mSetPos(&model, friend_pos[4]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlGinger);
    glDrawElements(GL_TRIANGLES, ginger_numind, GL_UNSIGNED_SHORT, 0);

    // farmer
    mIdent(&model);
    mSetPos(&model, friend_pos[5]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlFarmer);
    glDrawElements(GL_TRIANGLES, farmer_numind, GL_UNSIGNED_SHORT, 0);

    // foxy
    mIdent(&model);
    mSetPos(&model, friend_pos[6]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlFoxy);
    glDrawElements(GL_TRIANGLES, foxy_numind, GL_UNSIGNED_SHORT, 0);

    // frog
    mIdent(&model);
    mSetPos(&model, friend_pos[7]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlFrog);
    glDrawElements(GL_TRIANGLES, frog_numind, GL_UNSIGNED_SHORT, 0);

    // rat
    mIdent(&model);
    mSetPos(&model, friend_pos[8]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlRat);
    glDrawElements(GL_TRIANGLES, rat_numind, GL_UNSIGNED_SHORT, 0);

    // shroom
    mIdent(&model);
    mSetPos(&model, friend_pos[9]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlShroom);
    glDrawElements(GL_TRIANGLES, shroom_numind, GL_UNSIGNED_SHORT, 0);

    // robot
    mIdent(&model);
    mSetPos(&model, friend_pos[10]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlRobot);
    glDrawElements(GL_TRIANGLES, robot_numind, GL_UNSIGNED_SHORT, 0);

    // teddy
    mIdent(&model);
    mSetPos(&model, friend_pos[11]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlTeddy);
    glDrawElements(GL_TRIANGLES, teddy_numind, GL_UNSIGNED_SHORT, 0);

    // treeman
    mIdent(&model);
    mSetPos(&model, friend_pos[12]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlTreeman);
    glDrawElements(GL_TRIANGLES, treeman_numind, GL_UNSIGNED_SHORT, 0);

    // dragon
    mIdent(&model);
    mSetPos(&model, friend_pos[13]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlDragon);
    glDrawElements(GL_TRIANGLES, dragon_numind, GL_UNSIGNED_SHORT, 0);

    // banana
    mIdent(&model);
    mSetPos(&model, friend_pos[14]);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlBanana);
    glDrawElements(GL_TRIANGLES, banana_numind, GL_UNSIGNED_SHORT, 0);

    // octo
    mIdent(&model);
    mSetDir(&model, octo_dir);
    mSetPos(&model, octo_pos);
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlOcto);
    glDrawElements(GL_TRIANGLES, octo_numind, GL_UNSIGNED_SHORT, 0);
    if(octo_tgt != -1)
    {
        const float xm = (friend_pos[octo_tgt].x - octo_pos.x);
        const float ym = (friend_pos[octo_tgt].y - octo_pos.y);
        if(xm*xm + ym*ym > 0.0001f)
        {
            vec inc = friend_pos[octo_tgt];
            vSub(&inc, inc, octo_pos);
            inc.z = 0.f;
            vNorm(&inc);
            octo_dir = inc;
            vMulS(&inc, inc, 0.88888888f*dt);
            vAdd(&octo_pos, octo_pos, inc);
        }
        else if(friend_pos[octo_tgt].z < 0.f)
        {
            friend_pos[octo_tgt].z += 0.15f*dt;
        }
        else
        {
            octo_tgt = -1;
            updateTitle();
        }
    }
    else
    {
        uint fail = 1;
        for(uint i = 0; i < 15; i++)
        {
            if(friend_pos[i].z < -0.001f) // epsilon
            {
                octo_tgt = i;
                fail = 0;
                break;
            }
        }
        if(fail == 1){octo_tgt = -1;}
    }

    // ghosts
    for(uint i = 0; i < MAX_GHOSTS; i++)
    {
        // send ghost to another dimension? GHOST BUSTERS!?
        if(ghost_tgt[i] == -1)
        {
            if(ghost_opa[i] > 0.f)
            {
                ghost_opa[i] -= 0.42f * dt;
            }
            else
            {
                resetGhost(i);
                continue;
            }
            continue;
        }

        // target dead?
        if(friend_pos[ghost_tgt[i]].z <= -0.74f)
            retargetGhost(i); //ghost_tgt[i] = -1;

        // rise from the grave
        if(ghost_pos[i].z < -1.f){ghost_pos[i].z = -1.f;} // this is a quick fix to [1]
        if(ghost_pos[i].z < 0.f)
        {
            ghost_pos[i].z += 0.24f*dt; // [1] some how causes z to go way below -1 at times
        }
        else
        {
            const float xm = (-pp.x - ghost_pos[i].x);
            const float ym = (-pp.y - ghost_pos[i].y);
            const float xm1 = (friend_pos[ghost_tgt[i]].x - ghost_pos[i].x);
            const float ym1 = (friend_pos[ghost_tgt[i]].y - ghost_pos[i].y);
            const float fgd = xm1*xm1 + ym1*ym1;
            if(xm*xm + ym*ym > 0.17f && fgd > 0.1f)
            {
                vec inc = ghost_dir[i];
                vMulS(&inc, ghost_dir[i], 0.3f*dt);
                vAdd(&ghost_pos[i], ghost_pos[i], inc);
            }
            else if(fgd < 0.1f)
            {
                if(friend_pos[ghost_tgt[i]].z > -0.74f)
                {
                    friend_pos[ghost_tgt[i]].z -= 0.03f*dt;
                }
                else
                {
                    //ghost_tgt[i] = -1;
                    retargetGhost(i);
                    updateTitle();
                }
            }
        }

        // render
        mIdent(&model);
        mSetDir(&model, ghost_dir[i]);
        mSetPos(&model, ghost_pos[i]);
        mMul(&modelview, &model, &view);
        glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
        modelBind3(&mdlGhost);
        glDrawElements(GL_TRIANGLES, ghost_numind, GL_UNSIGNED_SHORT, 0);
    }
    
    // do the transparent ghosts now
    for(uint i = 0; i < MAX_GHOSTS; i++)
    {
        if(ghost_tgt[i] == -1)
        {
            if(ghost_opa[i] > 0.f)
            {
                ghost_opa[i] -= 0.42f * dt;
            }
            else
            {
                resetGhost(i);
                continue;
            }
            mIdent(&model);
            mSetDir(&model, ghost_dir[i]);
            mSetPos(&model, ghost_pos[i]);
            mMul(&modelview, &model, &view);
            glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
            modelBind3(&mdlGhost);
            glUniform1f(opacity_id, ghost_opa[i]);
            glEnable(GL_BLEND);
            glEnable(GL_CULL_FACE);
                glDrawElements(GL_TRIANGLES, ghost_numind, GL_UNSIGNED_SHORT, 0);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
        }
    }

    ///

    // render bonking tool
    mIdent(&model);
    vec ld = look_dir;
    ld.z = 0.f;
    vMulS(&ld, ld, 0.08f); // far from camera
    vec np = (vec){-pp.x, -pp.y, -pp.z};
    vAdd(&np, np, ld);

    // x offset
    vec vx = lookx;
    vMulS(&vx, vx, -0.08f);
    vAdd(&np, np, vx);

    // y offset
    vec vy = looky;
    vMulS(&vy, vy, 0.0788f); // 0.0388f or 0.0888f
    vAdd(&np, np, vy);

    // rotate to camera direction with slight offset and set pos
    mScale(&model, 0.6f, 0.6f,  0.6f);
    mRotZ(&model, -(xrot-PI));
    if(bonking == 1){mRotY(&model, -0.6f); mRotX(&model, -0.6f);}
    mSetPos(&model, np);

    // render it
    mMul(&modelview, &model, &view);
    glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
    modelBind3(&mdlClub);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, club_numind, GL_UNSIGNED_SHORT, 0);

    ///

    // render intro
    if(t < 5.f)
    {
        // shader program change
        shadeFullbright(&position_id, &projection_id, &modelview_id, &color_id, &opacity_id);
        glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);

        if(t < 4.f)
        {
            glUniform1f(opacity_id, 1.f);
        }
        else
        {
            glUniform1f(opacity_id, 1.f-(t-4.f));
        }
        glEnable(GL_BLEND);
        {
            vec ld = look_dir;
            vMulS(&ld, ld, 1.79f);
            vec np = (vec){-pp.x, -pp.y, -pp.z};
            vAdd(&np, np, ld);

            mIdent(&model);
            mSetPos(&model, np);
            mRotZ(&model, -xrot);
            mMul(&modelview, &model, &view);
            glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
            modelBind(&mdlIntro);
            glUniform3f(color_id, 0.854f, 0.335f, 1.f);
            glDrawElements(GL_TRIANGLES, intro_numind, GL_UNSIGNED_SHORT, 0);
        }
        {
            vec ld = look_dir;
            vMulS(&ld, ld, 1.8f);
            vec np = (vec){-pp.x, -pp.y, -pp.z};
            vAdd(&np, np, ld);

            vec vy = looky;
            vMulS(&vy, vy, -0.004f);
            vAdd(&np, np, vy);

            mIdent(&model);
            mSetPos(&model, np);
            mRotZ(&model, -xrot);
            mMul(&modelview, &model, &view);
            glUniformMatrix4fv(modelview_id, 1, GL_FALSE, (float*)&modelview.m[0][0]);
            modelBind(&mdlIntro);
            glUniform3f(color_id, 0.f, 0.f, 0.f);
            glDrawElements(GL_TRIANGLES, intro_numind, GL_UNSIGNED_SHORT, 0);
        }
        glDisable(GL_BLEND);
    }

    ///

    // display render
    glfwSwapBuffers(window);
}

//*************************************
// input
//*************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        if(     key == GLFW_KEY_A || key == GLFW_KEY_LEFT)  { keystate[0] = 1; }
        else if(key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { keystate[1] = 1; }
        else if(key == GLFW_KEY_W || key == GLFW_KEY_UP)    { keystate[2] = 1; }
        else if(key == GLFW_KEY_S || key == GLFW_KEY_DOWN)  { keystate[3] = 1; }
        else if(key == GLFW_KEY_LEFT_SHIFT ||
                key == GLFW_KEY_RIGHT_CONTROL)              { keystate[4] = 1;}
        else if(key == GLFW_KEY_N) // new game
        {
            timeTaken(0);
            char strts[16];
            timestamp(&strts[0]);
            printf("[%s] 🥶 Game End. 🥂 😎\n", strts);
            printf("[%s] 🫂 বন্ধুরা%u - 👻 বন্ধ %u\n", strts, friendsalive, ghostsbonked);
            printf("[%s] ⏳ Time-Taken: %s or %g Seconds 🫣\n\n", strts, tts, t-st);
            newGame(time(0));
        }
        else if(key == GLFW_KEY_ESCAPE) // toggle mouse focus
        {
            focus_cursor = 0;
#ifndef WEB
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwGetCursorPos(window, &lx, &ly);
#endif
        }
        else if(key == GLFW_KEY_F) // show average fps
        {
            if(t-lfct > 2.0)
            {
                char strts[16];
                timestamp(&strts[0]);
                printf("[%s] FPS: %g\n", strts, fc/(t-lfct));
                lfct = t;
                fc = 0;
            }
        }
    }
    else if(action == GLFW_RELEASE)
    {
        if(     key == GLFW_KEY_A || key == GLFW_KEY_LEFT)  { keystate[0] = 0; }
        else if(key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { keystate[1] = 0; }
        else if(key == GLFW_KEY_W || key == GLFW_KEY_UP)    { keystate[2] = 0; }
        else if(key == GLFW_KEY_S || key == GLFW_KEY_DOWN)  { keystate[3] = 0; }
        else if(key == GLFW_KEY_LEFT_SHIFT ||
                key == GLFW_KEY_RIGHT_CONTROL)              { keystate[4] = 0;}
    }
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        if(focus_cursor == 0)
        {
            focus_cursor = 1;
#ifndef WEB
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &lx, &ly);
#endif
            return;
        }
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            for(uint i = 0; i < MAX_GHOSTS; i++)
            {
                const float xm = (-pp.x - ghost_pos[i].x);
                const float ym = (-pp.y - ghost_pos[i].y);
                if(ghost_pos[i].z >= 0.f && xm*xm + ym*ym < 0.17f && ghost_tgt[i] != -1)
                {
                    ghost_tgt[i] = -1;
                    bonking = 1;
                    ghostsbonked++;
                    updateTitle();
                    return;
                }
            }
        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            mIdent(&projection);
            mPerspective(&projection, 30.0f, aspect, 0.01f, FAR_DISTANCE);
            glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
        }
    }
    else if(action == GLFW_RELEASE)
    {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            bonking = 0;
        }
        else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            mIdent(&projection);
            mPerspective(&projection, 60.0f, aspect, 0.01f, FAR_DISTANCE);
            glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
        }
    }
}
void window_size_callback(GLFWwindow* window, int width, int height)
{
    winw = width, winh = height;
    glViewport(0, 0, winw, winh);
    aspect = (float)winw / (float)winh;
    ww = winw, wh = winh;
    mIdent(&projection);
    mPerspective(&projection, 60.0f, aspect, 0.01f, FAR_DISTANCE);
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
}
#ifdef WEB
EM_BOOL emscripten_resize_event(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
{
    winw = uiEvent->documentBodyClientWidth;
    winh = uiEvent->documentBodyClientHeight;
    window_size_callback(window, winw, winh);
    emscripten_set_canvas_element_size("canvas", winw, winh);
    return EM_FALSE;
}
#endif

//*************************************
// process entry point
//*************************************
int main(int argc, char** argv)
{
    // allow custom msaa level & mouse sensitivity
    int msaa = 16;
    if(argc >= 2){msaa = atoi(argv[1]);}
    if(argc >= 3){sens = atof(argv[2]);}

    // help
    printf("----\n");
    printf("🧊 James 💎 William 💎 Fletcher 🧊 (github.com/mrbid) 🐧 (notabug.org/Vandarin) 🕷️\n");
    printf("%s - Save 💾 your 🐌 garden 🧸 friends 🫂 from the 👻 ghosts! 🥸\n", appTitle);
    printf("----\n");
#ifndef WEB
    printf("🤗 Two command line arguments, msaa 0-16, mouse sensitivity. 🫠\n");
    printf("e.g; ./aigeneratedgame 16 0.003\n");
    printf("----\n");
#endif
    printf("🍒 ESCAPE = Unlock Mouse 🍆\n");
    printf("🚶 W,A,S,D / Arrow Keys = Move 🍌\n");
    printf("🏃 L-SHIFT / R-CTRL = Sprint 🕺\n");
    printf("🐰 Left Click = BONK! 🥰\n");
    printf("🔭 Right Click = Zoom 🎁\n");
    printf("----\n");
    printf("🦾 Every 3D 🌐😭🤪🥴🤯 model ⚕️  was 🖖 generated 🕵️  by a 🧮 tensor operation 😱, 🤓 machine learning, 👾 artificial intelligence, ☠️  cyberdyne inc 💀, but a 🗿 human 🧬 👽 programmed 🪬 the 🛸 game 🕹️  and the ॐ lord 🙏 created the game concept. 🚀 https://meshy.ai ☎️  Keep in 🧠 mind I 📽️  projected 🤔 all the 🍿 textures 🍭 to Vertex 🎨 Colors 🧐, if I had not 🙅 done that 🤫 the AI 🧬🤖 generated 👯 models would have 👀 looked 🔎 a 👁️  lot 🌎 better. 🤯 Font 🫠 is 🪼 https://www.fontspace.com/slimespooky-font-f84976 🪼 by ✨ jadaakbal ✨ (jadaakbaaal@gmail.com) 📨.\n");
    printf("----\n");
    printf("🎄 Merry Christmas 2023! 🧙‍♂️  and a happy new year 2024! 🎅\n");
    printf("----\n");
    printf("ﮩ٨ـﮩﮩ٨ـ♡ﮩ٨ـﮩﮩ٨ـ %s 👁️⃤\n", glfwGetVersionString());
    printf("----\n");

    // init glfw
    if(!glfwInit()){printf("glfwInit() failed. 🤡\n"); exit(EXIT_FAILURE);}
#ifdef WEB
    double width, height;
    emscripten_get_element_css_size("body", &width, &height);
    winw = (uint)width, winh = (uint)height;
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, msaa);
    window = glfwCreateWindow(winw, winh, appTitle, NULL, NULL);
    if(!window)
    {
        printf("glfwCreateWindow() failed. 🤡\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    const GLFWvidmode* desktop = glfwGetVideoMode(glfwGetPrimaryMonitor());
#ifndef WEB
    glfwSetWindowPos(window, (desktop->width/2)-(winw/2), (desktop->height/2)-(winh/2)); // center window on desktop
#endif
    if(glfwRawMouseMotionSupported() == GLFW_TRUE){glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);} // raw input, since it's an FPS
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1); // 0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for adaptive vsync

    // set icon
    glfwSetWindowIcon(window, 1, &(GLFWimage){16, 16, (unsigned char*)icon_image});

//*************************************
// bind vertex and index buffers
//*************************************

    // ***** BIND INTRO *****
    esBind(GL_ARRAY_BUFFER, &mdlIntro.vid, intro_vertices, sizeof(intro_vertices), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlIntro.iid, intro_indices, sizeof(intro_indices), GL_STATIC_DRAW);

    // ***** BIND TERRAIN *****
    esBind(GL_ARRAY_BUFFER, &mdlTerrain.vid, terrain_vertices, sizeof(terrain_vertices), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlTerrain.iid, terrain_indices, sizeof(terrain_indices), GL_STATIC_DRAW);

    // ***** BIND SCENE *****
    esBind(GL_ARRAY_BUFFER, &mdlScene.vid, scene_vertices, sizeof(scene_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlScene.nid, scene_normals, sizeof(scene_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlScene.cid, scene_colors, sizeof(scene_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlScene.iid, scene_indices, sizeof(scene_indices), GL_STATIC_DRAW);

    // ***** BIND CLUB *****
    esBind(GL_ARRAY_BUFFER, &mdlClub.vid, club_vertices, sizeof(club_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlClub.nid, club_normals, sizeof(club_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlClub.cid, club_colors, sizeof(club_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlClub.iid, club_indices, sizeof(club_indices), GL_STATIC_DRAW);

    // ***** BIND GHOST *****
    esBind(GL_ARRAY_BUFFER, &mdlGhost.vid, ghost_vertices, sizeof(ghost_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlGhost.nid, ghost_normals, sizeof(ghost_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlGhost.cid, ghost_colors, sizeof(ghost_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlGhost.iid, ghost_indices, sizeof(ghost_indices), GL_STATIC_DRAW);

    // ***** BIND OCTO *****
    esBind(GL_ARRAY_BUFFER, &mdlOcto.vid, octo_vertices, sizeof(octo_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlOcto.nid, octo_normals, sizeof(octo_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlOcto.cid, octo_colors, sizeof(octo_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlOcto.iid, octo_indices, sizeof(octo_indices), GL_STATIC_DRAW);

    // ***** BIND BABY *****
    esBind(GL_ARRAY_BUFFER, &mdlBaby.vid, baby_vertices, sizeof(baby_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBaby.nid, baby_normals, sizeof(baby_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBaby.cid, baby_colors, sizeof(baby_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlBaby.iid, baby_indices, sizeof(baby_indices), GL_STATIC_DRAW);

    // ***** BIND BONES *****
    esBind(GL_ARRAY_BUFFER, &mdlBones.vid, bones_vertices, sizeof(bones_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBones.nid, bones_normals, sizeof(bones_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBones.cid, bones_colors, sizeof(bones_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlBones.iid, bones_indices, sizeof(bones_indices), GL_STATIC_DRAW);

    // ***** BIND BUNNY *****
    esBind(GL_ARRAY_BUFFER, &mdlBunny.vid, bunny_vertices, sizeof(bunny_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBunny.nid, bunny_normals, sizeof(bunny_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBunny.cid, bunny_colors, sizeof(bunny_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlBunny.iid, bunny_indices, sizeof(bunny_indices), GL_STATIC_DRAW);

    // ***** BIND CAT *****
    esBind(GL_ARRAY_BUFFER, &mdlCat.vid, cat_vertices, sizeof(cat_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlCat.nid, cat_normals, sizeof(cat_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlCat.cid, cat_colors, sizeof(cat_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlCat.iid, cat_indices, sizeof(cat_indices), GL_STATIC_DRAW);

    // ***** BIND GINGER *****
    esBind(GL_ARRAY_BUFFER, &mdlGinger.vid, ginger_vertices, sizeof(ginger_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlGinger.nid, ginger_normals, sizeof(ginger_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlGinger.cid, ginger_colors, sizeof(ginger_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlGinger.iid, ginger_indices, sizeof(ginger_indices), GL_STATIC_DRAW);

    // ***** BIND FARMER *****
    esBind(GL_ARRAY_BUFFER, &mdlFarmer.vid, farmer_vertices, sizeof(farmer_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlFarmer.nid, farmer_normals, sizeof(farmer_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlFarmer.cid, farmer_colors, sizeof(farmer_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlFarmer.iid, farmer_indices, sizeof(farmer_indices), GL_STATIC_DRAW);

    // ***** BIND FOXY *****
    esBind(GL_ARRAY_BUFFER, &mdlFoxy.vid, foxy_vertices, sizeof(foxy_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlFoxy.nid, foxy_normals, sizeof(foxy_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlFoxy.cid, foxy_colors, sizeof(foxy_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlFoxy.iid, foxy_indices, sizeof(foxy_indices), GL_STATIC_DRAW);

    // ***** BIND FROG *****
    esBind(GL_ARRAY_BUFFER, &mdlFrog.vid, frog_vertices, sizeof(frog_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlFrog.nid, frog_normals, sizeof(frog_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlFrog.cid, frog_colors, sizeof(frog_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlFrog.iid, frog_indices, sizeof(frog_indices), GL_STATIC_DRAW);

    // ***** BIND RAT *****
    esBind(GL_ARRAY_BUFFER, &mdlRat.vid, rat_vertices, sizeof(rat_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlRat.nid, rat_normals, sizeof(rat_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlRat.cid, rat_colors, sizeof(rat_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlRat.iid, rat_indices, sizeof(rat_indices), GL_STATIC_DRAW);

    // ***** BIND SHROOM *****
    esBind(GL_ARRAY_BUFFER, &mdlShroom.vid, shroom_vertices, sizeof(shroom_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlShroom.nid, shroom_normals, sizeof(shroom_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlShroom.cid, shroom_colors, sizeof(shroom_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlShroom.iid, shroom_indices, sizeof(shroom_indices), GL_STATIC_DRAW);

    // ***** BIND ROBOT *****
    esBind(GL_ARRAY_BUFFER, &mdlRobot.vid, robot_vertices, sizeof(robot_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlRobot.nid, robot_normals, sizeof(robot_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlRobot.cid, robot_colors, sizeof(robot_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlRobot.iid, robot_indices, sizeof(robot_indices), GL_STATIC_DRAW);

    // ***** BIND TEDDY *****
    esBind(GL_ARRAY_BUFFER, &mdlTeddy.vid, teddy_vertices, sizeof(teddy_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlTeddy.nid, teddy_normals, sizeof(teddy_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlTeddy.cid, teddy_colors, sizeof(teddy_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlTeddy.iid, teddy_indices, sizeof(teddy_indices), GL_STATIC_DRAW);

    // ***** BIND TREEMAN *****
    esBind(GL_ARRAY_BUFFER, &mdlTreeman.vid, treeman_vertices, sizeof(treeman_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlTreeman.nid, treeman_normals, sizeof(treeman_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlTreeman.cid, treeman_colors, sizeof(treeman_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlTreeman.iid, treeman_indices, sizeof(treeman_indices), GL_STATIC_DRAW);

    // ***** BIND DRAGON *****
    esBind(GL_ARRAY_BUFFER, &mdlDragon.vid, dragon_vertices, sizeof(dragon_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlDragon.nid, dragon_normals, sizeof(dragon_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlDragon.cid, dragon_colors, sizeof(dragon_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlDragon.iid, dragon_indices, sizeof(dragon_indices), GL_STATIC_DRAW);

    // ***** BIND BANANA *****
    esBind(GL_ARRAY_BUFFER, &mdlBanana.vid, banana_vertices, sizeof(banana_vertices), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBanana.nid, banana_normals, sizeof(banana_normals), GL_STATIC_DRAW);
    esBind(GL_ARRAY_BUFFER, &mdlBanana.cid, banana_colors, sizeof(banana_colors), GL_STATIC_DRAW);
    esBind(GL_ELEMENT_ARRAY_BUFFER, &mdlBanana.iid, banana_indices, sizeof(banana_indices), GL_STATIC_DRAW);

//*************************************
// compile & link shader programs
//*************************************
    makeFullbright();
    makeLambert3();

//*************************************
// configure render options
//*************************************
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.745f, 0.8863f, 0.f);

    shadeLambert3(&position_id, &projection_id, &modelview_id, &lightpos_id, &normal_id, &color_id, &opacity_id);
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&projection.m[0][0]);
    window_size_callback(window, winw, winh);

//*************************************
// execute update / render loop
//*************************************

    // init
    friend_pos[0]  = baby_pos;
    friend_pos[1]  = bones_pos;
    friend_pos[2]  = bunny_pos;
    friend_pos[3]  = cat_pos;
    friend_pos[4]  = ginger_pos;
    friend_pos[5]  = farmer_pos;
    friend_pos[6]  = foxy_pos;
    friend_pos[7]  = frog_pos;
    friend_pos[8]  = rat_pos;
    friend_pos[9]  = shroom_pos;
    friend_pos[10] = robot_pos;
    friend_pos[11] = teddy_pos;
    friend_pos[12] = treeman_pos;
    friend_pos[13] = dragon_pos;
    friend_pos[14] = banana_pos;
    newGame(NEWGAME_SEED);
    t = (float)glfwGetTime();
    lfct = t;

#ifdef WEB
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, emscripten_resize_event);
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while(!glfwWindowShouldClose(window)){main_loop();}
#endif

    // end
    timeTaken(0);
    char strts[16];
    timestamp(&strts[0]);
    printf("[%s] 🥶 Game End. 🥂 😎\n", strts);
    printf("[%s] 🫂 বন্ধুরা%u - 👻 বন্ধ %u\n", strts, friendsalive, ghostsbonked);
    printf("[%s] ⏳ Time-Taken: 🍒 %s 🤟 or ✌  %g 🚨 Seconds 🫣\n\n", strts, tts, t-st);

    // done
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
