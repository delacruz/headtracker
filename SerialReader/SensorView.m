//
//  SensorView.m
//  SerialReader
//
//  Created by Jason de la Cruz on 9/17/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "SensorView.h"
#import <GLUT/glut.h>

#define LIGHT_X_TAG 0
#define THETA_TAG 1
#define RADIUS_TAG 2

@implementation SensorView

@synthesize sensorHeading, sensorPitch, sensorRoll;

- (void)prepare
{
	NSLog(@"prepare");
	
	// The GL context must be active for these functions to have an effect
    NSOpenGLContext *openGLContext = [self openGLContext];
    [openGLContext makeCurrentContext];	
	
	// Configure the view
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	// Add some ambient lighting.
	GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0}; 
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	
	// Initialize the light
	GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	// and switch it on.
	glEnable(GL_LIGHT0);
	
	// Set the properties of the material under ambient light
	GLfloat mat[] = { 0.7, 0.1, 0.1, 1.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat);
	
	// Set the properties of the material under diffuse light
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);	
	
}

- (id)initWithCoder:(NSCoder *)c
{
	NSLog(@"initWithCoder:");
	self = [super initWithCoder:c];
	[self prepare];
	return self;
}


// Called when the view resizes
- (void)reshape
{
	NSLog(@"reshaping");
    NSRect rect = [self bounds];
    glViewport(0,0, rect.size.width, rect.size.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0,
                   rect.size.width/rect.size.height,
                   0.2, 7);
}
- (void)awakeFromNib
{

}

- (void)drawRect:(NSRect)r
{
    GLfloat lightPosition[] = {lightX, 3, 3, 0.0};
    GLUquadricObj *quadratic;
	quadratic=gluNewQuadric();
	gluQuadricNormals(quadratic, GLU_SMOOTH);
	gluQuadricTexture(quadratic, GL_TRUE);	
	
    // Clear the background
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);
    
    // Set the view point
    glMatrixMode(GL_MODELVIEW);
	
    glLoadIdentity();

    glTranslatef(0, 0, -2);
	
    // Put the light in place
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    
    if (!displayList)
    {
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE_AND_EXECUTE);
        
        // Draw the stuff
        glutSolidCone(0.5f, 1, 7, 32);
		glTranslatef(0, 0, -1);
		gluCylinder(quadratic,.2f,0.2f,1.0f,7,32);
        
        glEndList();
    } else {
		
		glRotatef(-sensorHeading,	0, 1, 0);
		glRotatef(-sensorPitch,		1, 0, 0);
		glRotatef( sensorRoll,		0, 0, 1);
		
        glCallList(displayList);
    }
    
    // Flush to screen
    glFinish();
}


@end


