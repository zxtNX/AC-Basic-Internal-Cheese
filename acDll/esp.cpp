#include "pch.h"
#include "esp.h"
#include "aimbot.h"
#include "globals.h"

#define screenWidth viewport[2]
#define screenHeight viewport[3]

Color color;
int viewport[4]{ 0 };
bool bEspStatus = false;

void SetupGL()
{
    // get the height and width of the screen
    glGetIntegerv(GL_VIEWPORT, viewport);

    // tell OpenGL we are abouit to do Projection math orthogonally
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    // switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

bool WorldToScreen(EntityInfo& info)
{
    static float* VPmatrix = (float*)(gModuleBaseAssaultCube + 0x101AE8);

    // Get clip values
    Vector4 clip;
    clip.x = info.entity->xHeadCoord * VPmatrix[0] + info.entity->yHeadCoord * VPmatrix[4] + info.entity->zHeadCoord * VPmatrix[8] + VPmatrix[12];
    clip.y = info.entity->xHeadCoord * VPmatrix[1] + info.entity->yHeadCoord * VPmatrix[5] + info.entity->zHeadCoord * VPmatrix[9] + VPmatrix[13];
    //clip.z = info.entity->xHeadCoord * VPmatrix[2] + info.entity->yHeadCoord * VPmatrix[6] + info.entity->zHeadCoord * VPmatrix[10] + VPmatrix[14]; // only for reference to know how matrix is done
    clip.w = info.entity->xHeadCoord * VPmatrix[3] + info.entity->yHeadCoord * VPmatrix[7] + info.entity->zHeadCoord * VPmatrix[11] + VPmatrix[15];

    // If behind the camera, don't update information
    if (clip.w < 0.1f)
        return false;

    // Normalize the clip values by dividing by w (perspective division)
    // This gives us a value from -1 to 1 to represent on our screen, but it's not pixel informations yet
    Vector4 NDC;
    NDC.x = clip.x / clip.w;
    NDC.y = clip.y / clip.w;
    //NDC.z = clip.z / clip.w;

    // Turn the normalized coordinates into pixel data
    info.screenX = (screenWidth / 2 * NDC.x) + (NDC.x + screenWidth / 2);
    info.screenY = (screenHeight / 2 * NDC.y) + (NDC.y + screenHeight / 2);

    return true;
}

void DrawEverything(std::vector<EntityInfo>& entityInfos, Entity* localPlayer, float maxFov) {
    // Make sure OpenGL is ready to draw with up-to-date information
    SetupGL();

    // Convertir MAX_FOV en radians
    float maxFovRadians = static_cast<float>(maxFov * (gPI / 180.0));

    // Dessiner le cercle au centre de l'écran avec la taille basée sur maxFov
    float centerX = static_cast<float>(screenWidth / 2);
    float centerY = static_cast<float>(screenHeight / 2);
    float circleRadius = static_cast<float>(tan(maxFovRadians / 2.0) * screenWidth / 2.0); // Calcul du rayon en fonction de l'angle de champ de vision
    DrawCenterCircle(centerX, centerY, circleRadius, 36);

    // Loop through every alive/visible entity and draw boxes around them
    for (auto& info : entityInfos) {
        info.isVisible = WorldToScreen(info); // Convert entity coordinates to screen coordinates and return true if enemy visible
        if (!info.entity->bDead && info.isVisible) {
            DrawBoxAndSnaplines(info, localPlayer); // Draw box w/ health using screen coordinates and draw snaplines
        }
    }
}

void DrawBoxAndSnaplines(EntityInfo& info, Entity* localPlayer)
{
    // Color for boxes (team = green, enemies = red/grey)
    if (info.entity->team != localPlayer->team)
        glColor3ub(color.red[0], color.red[1], color.red[2]);

    if (info.entity->team == localPlayer->team)
        glColor3ub(color.green[0], color.green[1], color.green[2]);

    if (info.entity->team != localPlayer->team && !(info.isTargetable))
        glColor3ub(color.grey[0], color.grey[1], color.grey[2]);

    // Adjusted distance
    static float gameUnits = 950.0f;
    float distanceAdjustment = gameUnits / info.distanceFromPlayer;

    // Calculate adjustment factor based on target resolution (2560x1440)
    float targetWidth = 2560.0f;
    float targetHeight = 1440.0f;
    float widthAdjustment = screenWidth / targetWidth;
    float heightAdjustment = screenHeight / targetHeight;
    float adjustmentFactor = min(widthAdjustment, heightAdjustment);

    // Calculate direction vector from player to target
    Vector3 playerToTarget;
    playerToTarget.x = info.entity->xHeadCoord - localPlayer->xHeadCoord;
    playerToTarget.y = info.entity->yHeadCoord - localPlayer->yHeadCoord;
    playerToTarget.z = info.entity->zHeadCoord - localPlayer->zHeadCoord;

    // Normalize direction vector
    float length = static_cast<float>(sqrt(playerToTarget.x * playerToTarget.x + playerToTarget.y * playerToTarget.y + playerToTarget.z * playerToTarget.z));
    playerToTarget.x /= length;
    playerToTarget.y /= length;
    playerToTarget.z /= length;

    // Convert pitch and yaw angles to radians
    float pitchRad = localPlayer->pitch * (gPI / 180.0f); // Convert degrees to radians
    float yawRad = localPlayer->yaw * (gPI / 180.0f); // Convert degrees to radians

    // Calculate direction vector from pitch and yaw
    Vector3 viewDirection;
    viewDirection.x = static_cast<float>(cos(yawRad) * cos(pitchRad));
    viewDirection.y = static_cast<float>(sin(yawRad) * cos(pitchRad));
    viewDirection.z = static_cast<float>(sin(pitchRad));

    // Normalize direction vector
    length = static_cast<float>(sqrt(viewDirection.x * viewDirection.x + viewDirection.y * viewDirection.y + viewDirection.z * viewDirection.z));
    viewDirection.x /= length;
    viewDirection.y /= length;
    viewDirection.z /= length;

    // Calculate dot product between player's view direction and target direction
    float dotProduct = playerToTarget.x * viewDirection.x +
        playerToTarget.y * viewDirection.y +
        playerToTarget.z * viewDirection.z;

    // Adjust distance based on player's view angle relative to target
    float angleFactor = max(0.5f, abs(dotProduct)); // Minimum value to prevent excessive reduction
    distanceAdjustment *= angleFactor;

    // Apply adjustment factor to the distance
    distanceAdjustment *= adjustmentFactor;

    // Adjust rectangle size towards the top
    float topAdjustment = 1.5f * distanceAdjustment;

    // Draw a box with 2.0f sized lines
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(info.screenX - 2.6f * distanceAdjustment, info.screenY + topAdjustment);
    glVertex2f(info.screenX + 2.6f * distanceAdjustment, info.screenY + topAdjustment);
    glVertex2f(info.screenX + 2.6f * distanceAdjustment, info.screenY - 9.5f * distanceAdjustment);
    glVertex2f(info.screenX - 2.6f * distanceAdjustment, info.screenY - 9.5f * distanceAdjustment);
    glEnd();

    // Get the bottom Y coordinate of the box
    float boxBottomY = info.screenY - 9.5f * distanceAdjustment; // Adjust this calculation based on our existing box drawing code

    DrawHealthBar(info, localPlayer, distanceAdjustment, topAdjustment);

    // Call DrawSnaplines if the target is an enemy
    if (info.entity->team != localPlayer->team)
        DrawSnaplines(info, localPlayer, boxBottomY);
}

void DrawSnaplines(EntityInfo& info, Entity* localPlayer, float boxBottomY)
{
    // Si la cible est un ennemi, afficher la snapline
    if (info.entity->team != localPlayer->team) {
        // Couleur des lignes (bleu par défaut)
        glColor3ub(255, 0, 0);

        // Dessiner la ligne de l'ennemi à la position du joueur local
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(static_cast<float>(screenWidth / 2), 0); // Point de départ (milieu en bas de l'écran)
        glVertex2f(info.screenX, boxBottomY);  // Point d'arrivée (position de l'ennemi)
        glEnd();
    }
}

void DrawHealthBar(EntityInfo& info, Entity* localPlayer, float distanceAdjustment, float topAdjustment)
{
    const int maxHealth = 100;
    const int numberOfSlices = 10; // CHaque 10% HP donc 10*10 = 100

    // Calcul de la hauteur de la barre de vie basée sur la santé actuelle
    float healthPercentage = (float)info.entity->health / maxHealth;
    float barHeight = 11.0f * distanceAdjustment;  // Hauteur totale de la boîte
    float healthBarHeight = barHeight * healthPercentage;  // Hauteur de la barre de vie proportionnelle à la santé

    // Définition de la largeur et la position de la barre de vie
    float barWidth = 0.5f * distanceAdjustment;
    float barXLeft = info.screenX + 2.65f * distanceAdjustment + 0.1f * distanceAdjustment;
    float barXRight = barXLeft + barWidth;
    float barBottomY = info.screenY - 9.5f * distanceAdjustment; // Bas de la barre de santé totale
    float barTopY = barBottomY + healthBarHeight;  // Haut de la barre de vie en fonction de la santé actuelle

    // Ajustement pour l'encadré blanc
    float outlineTopY = barBottomY + barHeight;  // Le haut de l'encadré doit être au top de la barre totale

    // Dessiner les tranches sur la barre de vie
    glColor3ub(255, 255, 255); // Utilise la couleur blanche pour les lignes de séparation
    float sliceHeight = barHeight / numberOfSlices; // Hauteur de chaque tranche
    for (int i = 1; i < numberOfSlices; ++i) { // Commence à 1 pour ne pas dessiner au bas de la barre
        float sliceY = barBottomY + sliceHeight * i;
        glBegin(GL_LINES);
        glVertex2f(barXLeft, sliceY);
        glVertex2f(barXRight, sliceY);
        glEnd();
    }

    // Définir la couleur pour le contour (blanc)
    glColor3ub(255, 255, 255);

    // Dessiner le contour extérieur de la barre
    glLineWidth(0.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(barXLeft, outlineTopY);  // Haut gauche de l'encadré
    glVertex2f(barXRight, outlineTopY);  // Haut droit de l'encadré
    glVertex2f(barXRight, barBottomY);  // Bas droit de l'encadré
    glVertex2f(barXLeft, barBottomY);  // Bas gauche de l'encadré
    glEnd();

    // Définir la couleur de la barre de vie
    if (healthPercentage > 0.666)
        glColor3ub(0, 255, 0);  // Vert pour la santé élevée
    else if (healthPercentage > 0.333)
        glColor3ub(255, 127, 0);  // Orange pour la santé assez faible
    else
        glColor3ub(255, 0, 0);  // Rouge pour la santé faible

    // Dessiner la barre de vie
    glBegin(GL_QUADS);
    glVertex2f(barXLeft, barTopY);
    glVertex2f(barXRight, barTopY);
    glVertex2f(barXRight, barBottomY);
    glVertex2f(barXLeft, barBottomY);
    glEnd();
}

void DrawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments); // Get the current angle
        float x = r * cosf(theta);  // Calculate the x component
        float y = r * sinf(theta);  // Calculate the y component
        glVertex2f(x + cx, y + cy); // Output vertex
    }
    glEnd();
}

void DrawCenterCircle(float cx, float cy, float r, int num_segments) {
    // Déplacer le point de référence vers le centre de l'écran
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(cx, cy, 0.0f);

    // Dessiner le cercle
    glColor3ub(255, 255, 255); // Couleur blanche pour le cercle
    DrawCircle(0.0f, 0.0f, r, num_segments);

    // Rétablir la matrice de projection
    glPopMatrix();
}
