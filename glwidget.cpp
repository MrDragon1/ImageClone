#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent):
    QGLWidget(parent),
    tex_tar(QOpenGLTexture::Target2D),
    tex_src(QOpenGLTexture::Target2D),
    tex_weight(QOpenGLTexture::Target2D),
    tex_boundarycoords(QOpenGLTexture::Target2D)
{
    target_w = 400;
    target_h = 400;

    init = false;
    target = GL_TEXTURE_RECTANGLE_ARB;//GL_TEXTURE_2D;
    selection.vertices = 0;

    selection.tx = 0;
    selection.ty = 0;
    selection.dx = 0;
    selection.dy = 0;

    sourceImage = nullptr;
    targetImage = nullptr;

}


void GLWidget::updateSelection(std::vector<CDTPoint> & boundaryVector, QPolygonF & selectionPoly, bool reset)
{

    // Copy the data (we needed it in order to change the settings on the fly)
    //this->selectionPoly = selectionPoly;
    this->boundaryVector = boundaryVector;

    mesh = new Mesh(boundaryVector);
    selection.boundarySize = boundaryVector.size();
    selection.numPoints = mesh->vertex_list.size();
    selection.numTriangles = mesh->face_vertex_index.size();

    selection.vertices = new Point[selection.numPoints];
    selection.boundaryCoords = new Point[selection.boundarySize];
    selection.weightsTex = new float[selection.boundarySize * selection.numPoints];
    selection.triangles = new Triangles[selection.numTriangles];
    selection.boundaryTex = new float[selection.boundarySize * 2];
//    QFile file("./data.txt");
//    if(! file.open(QIODevice::WriteOnly|QIODevice::Text))  //
//    {
//        QMessageBox::critical(this,"错误","文件打开失败，信息没有保存！","确定");
//        return;
//    }

//    QTextStream out(&file);//写入

    for(int i = 0;i<selection.numPoints;i++)
    {
        selection.vertices[i].x = mesh->vertex_list[i].x();
        selection.vertices[i].y = mesh->vertex_list[i].y();
//        out << selection.vertices[i].x <<" "<< selection.vertices[i].y<<endl;
    }
//    out<<"########"<<endl;
    for(int i = 0;i<selection.numTriangles;i++)
    {
        selection.triangles[i].p1 = mesh->face_vertex_index[i][0];
        selection.triangles[i].p2 = mesh->face_vertex_index[i][1];
        selection.triangles[i].p3 = mesh->face_vertex_index[i][2];
//        out <<selection.triangles[i].p1<<" "<<selection.triangles[i].p2<<" "<<selection.triangles[i].p3<<endl;
    }
//    out<<"########"<<endl;
    for(int i = 0;i<selection.boundarySize;i++)
    {
        selection.boundaryCoords[i].x = boundaryVector[i].x();
        selection.boundaryCoords[i].y = boundaryVector[i].y();
        selection.boundaryTex[i] = boundaryVector[i].x();
        selection.boundaryTex[i + selection.boundarySize] = boundaryVector[i].y();
//        out << boundaryVector[i].x() << " " << boundaryVector[i].y()<<endl;
    }
//    file.close();
//    qDebug()<<"file write done";
    for(size_t i = 0;i<selection.numPoints;i++)
    {
        float sum = 0;
        for(size_t j = 0;j<selection.boundarySize;j++)
        {
            size_t index = i * selection.boundarySize + j;
            Point x,v1,v2,v3;
            x = selection.vertices[i];
            v1 = selection.boundaryCoords[j-1];
            v2 = selection.boundaryCoords[j];
            v3 = selection.boundaryCoords[j+1];
            float w = CalcWeight(x,v1,v2,v3);
            sum += w;
            selection.weightsTex[index] = w;
        }
        for(size_t j = 0;j<selection.boundarySize;j++)
        {
            size_t index = i * selection.boundarySize + j;
            selection.weightsTex[index] /= sum;
        }
    }

    selection.diff.resize(selection.boundarySize);
    for(int i = 0;i<selection.boundarySize;i++) selection.diff[i].resize(3);

    glGenTextures(1, &tex_boundarycoords_id);
    glBindTexture(target, tex_boundarycoords_id);
    // set the texture wrapping parameters
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // set texture filtering parameters
    glTexParameteri(target,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(target,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(target, 0, GL_RED, 2,selection.boundarySize, 0, GL_RED, GL_FLOAT, selection.boundaryTex);

    glGenTextures(1, &tex_weight_id);
    glBindTexture(target, tex_weight_id);
    // set the texture wrapping parameters
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // set texture filtering parameters
    glTexParameteri(target,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(target,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(target, 0, GL_RED,selection.boundarySize,selection.numPoints, 0, GL_RED, GL_FLOAT, selection.weightsTex);

    init = true;
}

cv::Mat QImage2cvMat(QImage image)
{
    using namespace cv;
    cv::Mat mat;
    qDebug() << image.format();
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

void GLWidget::OriginalBlend()
{
    qDebug()<<"begin blend";
    float fx=selection.tx+selection.dx;
    float fy=selection.ty+selection.dy;
    QImage tarimg = targetImage->copy();//->mirrored();
    QImage srcimg = sourceImage->copy();//->mirrored();
    tarimg.save("BlendImagebefore","jpg",100);

    std::vector<int> diff;
    for(size_t k = 0; k < selection.boundarySize; k++)
    {

        int i = selection.boundaryCoords[k].y,j = selection.boundaryCoords[k].x;
        QColor t = tarimg.pixelColor(j + fx,tarimg.height() - i - fy);
        QColor s = srcimg.pixelColor(j ,srcimg.height() - i);
        diff.push_back(t.red() - s.red());
        diff.push_back(t.green() - s.green());
        diff.push_back(t.blue() - s.blue());
    }

    std::vector<std::vector<float>> tri_mesh_vertex_R;
    for (size_t vi = 0; vi < selection.numPoints; ++vi)
    {
        std::vector<float> r(3,0);
        for(size_t pi = 0; pi < selection.boundarySize; pi++)
        {
            float w = selection.weightsTex[vi * selection.boundarySize + pi];
            r[0] += w * diff[pi * 3 + 0];
            r[1] += w * diff[pi * 3 + 1];
            r[2] += w * diff[pi * 3 + 2];
        }
        tri_mesh_vertex_R.push_back(r);
    }
    Point v1, v2, v3;
    int ind1, ind2, ind3;
    for (int i=0; i<selection.numTriangles; i++){
        ind1 = selection.triangles[i].p1;
        ind2 = selection.triangles[i].p2;
        ind3 = selection.triangles[i].p3;

        v1 = selection.vertices[ind1];
        v2 = selection.vertices[ind2];
        v3 = selection.vertices[ind3];


        for (size_t ri = 0; ri < srcimg.height(); ++ri)
        {
           for (size_t ci = 0; ci < srcimg.width(); ++ci)
           {
               if(!PointinTriangle(v1,v2,v3,Point(ci,ri)))
               {
                   continue;
               }

               std::vector<double> r(3, 0);
               for(int bi = 0; bi < 3; bi++)
               {
                   vector3d p0(v1.x, v1.y, tri_mesh_vertex_R[ind1][bi]);
                   vector3d p1(v2.x, v2.y, tri_mesh_vertex_R[ind2][bi]);
                   vector3d p2(v3.x, v3.y, tri_mesh_vertex_R[ind3][bi]);
                   vector3d vp(ci, ri, 0);

                   vp.CalPlanePointZ(p0, p1, p2);
                   r[bi] = vp.z;
               }

               QColor s(srcimg.pixelColor(ci,srcimg.height() - ri).red() + r[0],
                       srcimg.pixelColor(ci,srcimg.height() - ri).green() + r[1],
                       srcimg.pixelColor(ci,srcimg.height() - ri).blue() + r[2]);

               s.setRed(std::min(std::max(s.red(),0),255));
               s.setGreen(std::min(std::max(s.green(),0),255));
               s.setBlue(std::min(std::max(s.blue(),0),255));
               tarimg.setPixelColor(ci + fx,tarimg.height() - ri - fy,s);
           }
        }
    }
    tarimg.save("BlendImage","jpg",100);
    qDebug()<<"End blend";
}

bool GLWidget::PointinTriangle(Point A,Point B,Point C,Point P)
{
    Point v0 = C - A ;
    Point v1 = B - A ;
    Point v2 = P - A ;

    float dot00 = v0.Dot(v0) ;
    float dot01 = v0.Dot(v1) ;
    float dot02 = v0.Dot(v2) ;
    float dot11 = v1.Dot(v1) ;
    float dot12 = v1.Dot(v2) ;

    float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01) ;

    float u = (dot11 * dot02 - dot01 * dot12) * inverDeno ;
    if (u < 0 || u > 1) // if u out of range, return directly
    {
        return false ;
    }

    float v = (dot00 * dot12 - dot01 * dot02) * inverDeno ;
    if (v < 0 || v > 1) // if v out of range, return directly
    {
        return false ;
    }

    return u + v <= 1 ;
}

double GLWidget::CalcWeight(Point x,Point v1,Point v2,Point v3)
{
    double l12,l1,l2,l3,l23;
    double c1,c2,tan1,tan2;
    l12 = (v1-v2).Mod();
    l1 = (x-v1).Mod();
    l2 = (x-v2).Mod();
    l3 = (x-v3).Mod();
    l23 = (v2-v3).Mod();
    if(!(l12 && l1 && l2 && l3 && l23)) return 0;

    c1 = (l1*l1+l2*l2-l12*l12)/(2*l1*l2);
    c2 = (l3*l3+l2*l2-l23*l23)/(2*l3*l2);
    tan1 = tan(acos(c1)/2);
    tan2 = tan(acos(c2)/2);

    double w_a = tan1 + tan2;
    double w_b = l2;
    if(std::isnan(w_a/w_b)) return 0;
    return  w_a / w_b;
}

void GLWidget::bindTarget(){

    tex_tar.create();
    tex_tar.setData(targetImage->mirrored());
    tex_tar.setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
    tex_tar.setWrapMode(QOpenGLTexture::DirectionS,QOpenGLTexture::Repeat);
    tex_tar.setWrapMode(QOpenGLTexture::DirectionT,QOpenGLTexture::Repeat);
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex_tar_id);
    glBindTexture(target, tex_tar_id);
    // set the texture wrapping parameters
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(target, 0, GL_RGBA, targetImage->width(),targetImage->height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, targetImage->mirrored().bits());
    qDebug()<<"bind target done";
//    float* data = new float[32*32*4];
//    glReadPixels(0,0,32,32,GL_RGBA,GL_FLOAT,data);
//    for(int i = 0;i<32*32;i+=4)
//        qDebug()<<data[i]<<" "<<data[i+1]<<" "<<data[i+2]<<" "<<data[i+3]<<" ";
}

void GLWidget::bindSource(){
    tex_src.create();
    tex_src.setData(sourceImage->mirrored());
    tex_src.setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
    tex_src.setWrapMode(QOpenGLTexture::DirectionS,QOpenGLTexture::Repeat);
    tex_src.setWrapMode(QOpenGLTexture::DirectionT,QOpenGLTexture::Repeat);
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex_src_id);
    glBindTexture(target, tex_src_id);
    // set the texture wrapping parameters
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(target, 0, GL_RGBA, sourceImage->width(),sourceImage->height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sourceImage->mirrored().bits());
    qDebug()<<"bind source done";
    //qDebug()<<sourceImage->format()<<" "<<targetImage->format();
}

void GLWidget::setTargetImage(const QImage & image){

    if (targetImage)
        delete targetImage;

    targetImage = new QImage(image.copy());
    targetImage->save("./targetImage","jpg",100);
    target_w = image.width();
    target_h = image.height();
    bindTarget();

}

void GLWidget::setSourcePatch(const QImage & image)
{
    if (sourceImage)
        delete sourceImage;

    sourceImage = new QImage(image.copy());
    sourceImage->save("./sourceImage","jpg",100);
    selection_w = image.width();
    selection_h = image.height();
    source_h = selection_h;
    source_w = selection_w;
    bindSource();
}

void GLWidget::initializeGL(){

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);

    glEnable(target);

    loadShaders();

    VBO = new QOpenGLBuffer();
    VBO->create();
    VAO = new QOpenGLVertexArrayObject();
    VAO->create();

    qDebug()<<"test point initGL ";
    update();
}

void GLWidget::loadShaders()
{
    QString vertexcode,fragmentcode;
    QFile v_file(":/src/image.vert");
    QFile f_file(":/src/image.frag");
    if(v_file.open(QFile::ReadOnly|QFile::Text)&&f_file.open(QFile::ReadOnly|QFile::Text)){
        vertexcode = v_file.readAll();
        fragmentcode = f_file.readAll();
    }
    else {
        qWarning("shader file open failed");
        return ;
    }
    v_file.close();
    f_file.close();

    program = new QOpenGLShaderProgram(this);

    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexcode)) {
        qWarning("Vertex shader compilation failed");
    }

    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentcode)) {
        qWarning("Fragment shader compilation failed");
    }

    if (!program->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << program->log();
        delete program;
    }

    QString bgvertexcode,bgfragmentcode;
    QFile bv_file(":/bg.vert");
    QFile bf_file(":/bg.frag");
    if(bv_file.open(QFile::ReadOnly|QFile::Text)&&bf_file.open(QFile::ReadOnly|QFile::Text)){
        bgvertexcode = bv_file.readAll();
        bgfragmentcode = bf_file.readAll();
    }
    else {
        qWarning("shader file open failed");
        return ;
    }
    bv_file.close();
    bf_file.close();
    bgprogram = new QOpenGLShaderProgram(this);

    if (!bgprogram->addShaderFromSourceCode(QOpenGLShader::Vertex, bgvertexcode)) {
        qWarning("Vertex shader compilation failed");
    }

    if (!bgprogram->addShaderFromSourceCode(QOpenGLShader::Fragment, bgfragmentcode)) {
        qWarning("Fragment shader compilation failed");
    }

    if (!bgprogram->link()) {
        qWarning("Failed to compile and link shader program");
        qWarning("Shader program log:");
        qWarning() << bgprogram->log();
        delete bgprogram;
    }
}

void GLWidget::paintGL(){
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(target);
    bgprogram->bind();
//    QVector<float> vertices={
//            // 位置                  //纹理坐标
//             1.0f, -1.0f,       1.0f, 0.0f,
//            -1.0f, -1.0f,       0.0f, 0.0f,
//            -1.0f,  1.0f,       0.0f, 1.0f,
//             1.0f,  1.0f,       1.0f, 1.0f,
//        };
    QVector<float> vertices={
            // 位置                  //纹理坐标
             1.0f, -1.0f,       (float)target_w, 0.0f,
            -1.0f, -1.0f,       0.0f, 0.0f,
            -1.0f,  1.0f,       0.0f, (float)target_h,
             1.0f,  1.0f,       (float)target_w, (float)target_h,
        };
    QOpenGLVertexArrayObject::Binder vaoBind(VAO);

    VBO->create();
    VBO->bind();
    VBO->allocate(vertices.data(),sizeof(float)*vertices.size());

    int attr = -1;
    attr = bgprogram->attributeLocation("aPos");
    bgprogram->setAttributeBuffer(attr, GL_FLOAT, 0, 2, sizeof(GLfloat) * 4);
    bgprogram->enableAttributeArray(attr);

    attr = bgprogram->attributeLocation("TexCoords");
    bgprogram->setAttributeBuffer(attr, GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 4);
    bgprogram->enableAttributeArray(attr);
    VBO->release();


    glActiveTexture(GL_TEXTURE0_ARB);
    glBindTexture(target, tex_tar_id);
    //tex_tar.bind(0);
    bgprogram->setUniformValue("texture0",0);

    QOpenGLVertexArrayObject::Binder vaoBind1(VAO);
    glDrawArrays(GL_POLYGON,0,4);

    bgprogram->release();


    // Transform the selection patch
    //glTranslatef(selection.tx+selection.dx, selection.ty+selection.dy, 0);

    glEnable(target);
    paintSelection();
    //paintMesh();
    //paintDot();
}

void GLWidget::paintDot()
{
    if (!init)
        return;
    float fx=selection.tx+selection.dx;
    float fy=selection.ty+selection.dy;
    int ind1, ind2, ind3;
    Point v1, v2, v3;

    program->bind();
    glEnable(target);

    glActiveTexture(GL_TEXTURE0_ARB);
    glBindTexture(target, tex_tar_id);

    glActiveTexture(GL_TEXTURE1_ARB);
    glBindTexture(target, tex_src_id);

    //tex_tar.bind(0);
    program->setUniformValue("tex_tar",0);

    //tex_src.bind(1);
    program->setUniformValue("tex_src",1);

    glActiveTexture(GL_TEXTURE2_ARB);
    glBindTexture(target, tex_weight_id);
    program->setUniformValue("tex_weight",2);

    glActiveTexture(GL_TEXTURE3_ARB);
    glBindTexture(target, tex_boundarycoords_id);
    program->setUniformValue("tex_boundarycoords",3);

    program->setUniformValue("boundarysize",(int)selection.boundarySize);
    for (int i=0; i<selection.numTriangles; i++){

        glBegin(GL_TRIANGLES);
        ind1 = selection.triangles[i].p1;
        ind2 = selection.triangles[i].p2;
        ind3 = selection.triangles[i].p3;

        v1 = selection.vertices[ind1];
        v2 = selection.vertices[ind2];
        v3 = selection.vertices[ind3];

        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,v1.x + fx,v1.y + fy);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,v1.x,v1.y);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,0,ind1);
        glMultiTexCoord2fARB(GL_TEXTURE3_ARB,0,0);
        glVertex2f(v1.x + fx ,v1.y + fy);

        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,v2.x + fx,v2.y + fy);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,v2.x,v2.y);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,0,ind2);
        glMultiTexCoord2fARB(GL_TEXTURE3_ARB,0,0);
        glVertex2f(v2.x + fx, v2.y + fy);

        glMultiTexCoord2fARB(GL_TEXTURE0_ARB,v3.x + fx,v3.y + fy);
        glMultiTexCoord2fARB(GL_TEXTURE1_ARB,v3.x,v3.y);
        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,0,ind3);
        glMultiTexCoord2fARB(GL_TEXTURE3_ARB,0,0);
        glVertex2f(v3.x + fx, v3.y + fy);

        glEnd();
    }
}

void GLWidget::paintSelection(){

    if (!init)
        return;
    program->bind();

    float fx=selection.tx+selection.dx;
    float fy=selection.ty+selection.dy;

    Point v1, v2, v3;
    int ind1, ind2, ind3;

    QImage tarimg = targetImage->mirrored();
    QImage srcimg = sourceImage->mirrored();
    float *data = new float[selection.boundarySize * 6];
    for(size_t k = 0; k < selection.boundarySize; k++)
    {
        int i = selection.boundaryCoords[k].y,j = selection.boundaryCoords[k].x;
        QColor t = tarimg.pixelColor(j + fx,i + fy);
        QColor s = srcimg.pixelColor(j ,i);
        data[i] = (float)(t.red());
        data[i + selection.boundarySize] = (float)(t.green());
        data[i + selection.boundarySize * 2] = (float)(t.blue());
        data[i + selection.boundarySize * 3] = (float)(s.red());
        data[i + selection.boundarySize * 4] = (float)(s.green());
        data[i + selection.boundarySize * 5] = (float)(s.blue());
    }

    std::vector<int> diff;
    for(size_t k = 0; k < selection.boundarySize; k++)
    {

        int i = selection.boundaryCoords[k].y,j = selection.boundaryCoords[k].x;
        QColor t = tarimg.pixelColor(j + fx,i + fy);
        QColor s = srcimg.pixelColor(j , i);
        diff.push_back(t.red() - s.red());
        diff.push_back(t.green() - s.green());
        diff.push_back(t.blue() - s.blue());
    }

    std::vector<std::vector<float>> tri_mesh_vertex_R;
    for (size_t vi = 0; vi < selection.numPoints; ++vi)
    {
        std::vector<float> r(3,0);
        for(size_t pi = 0; pi < selection.boundarySize; pi++)
        {
            float w = selection.weightsTex[vi * selection.boundarySize + pi];
            r[0] += w * diff[pi * 3 + 0];
            r[1] += w * diff[pi * 3 + 1];
            r[2] += w * diff[pi * 3 + 2];
        }
        tri_mesh_vertex_R.push_back(r);
    }

    //qDebug()<<data[10]<<" "<<data[selection.boundarySize + 10]<<" "<<data[selection.boundarySize*2 + 10];
    GLuint tmptex;
    glGenTextures(1, &tmptex);
    glBindTexture(target, tmptex);
    // set the texture wrapping parameters
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_NEAREST);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_NEAREST);
    // set texture filtering parameters
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(target, 0, GL_LUMINANCE32F_ARB,6,selection.boundarySize, 0, GL_LUMINANCE, GL_FLOAT, data);


    std::vector<float> vertices;
    for (int i=0; i<selection.numTriangles; i++){
        ind1 = selection.triangles[i].p1;
        ind2 = selection.triangles[i].p2;
        ind3 = selection.triangles[i].p3;

        v1 = selection.vertices[ind1];
        v2 = selection.vertices[ind2];
        v3 = selection.vertices[ind3];

        vertices.push_back(v1.x/target_w);vertices.push_back(v1.y/target_h);vertices.push_back(v1.x);vertices.push_back(v1.y);vertices.push_back(ind1);
        vertices.push_back(tri_mesh_vertex_R[ind1][0]);vertices.push_back(tri_mesh_vertex_R[ind1][1]);vertices.push_back(tri_mesh_vertex_R[ind1][2]);

        vertices.push_back(v2.x/target_w);vertices.push_back(v2.y/target_h);vertices.push_back(v2.x);vertices.push_back(v2.y);vertices.push_back(ind2);
        vertices.push_back(tri_mesh_vertex_R[ind2][0]);vertices.push_back(tri_mesh_vertex_R[ind2][1]);vertices.push_back(tri_mesh_vertex_R[ind2][2]);

        vertices.push_back(v3.x/target_w);vertices.push_back(v3.y/target_h);vertices.push_back(v3.x);vertices.push_back(v3.y);vertices.push_back(ind3);
        vertices.push_back(tri_mesh_vertex_R[ind3][0]);vertices.push_back(tri_mesh_vertex_R[ind3][1]);vertices.push_back(tri_mesh_vertex_R[ind3][2]);
    }


    QOpenGLVertexArrayObject::Binder vaoBind(VAO);

    VBO->create();
    VBO->bind();
    VBO->allocate(vertices.data(),sizeof(float)*vertices.size());

    int attr = -1;
    attr = program->attributeLocation("aPos");
    program->setAttributeBuffer(attr, GL_FLOAT, 0, 2, sizeof(GLfloat) * 8);
    program->enableAttributeArray(attr);

    attr = program->attributeLocation("rPos");
    program->setAttributeBuffer(attr, GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 8);
    program->enableAttributeArray(attr);

    attr = program->attributeLocation("index");
    program->setAttributeBuffer(attr, GL_FLOAT, sizeof(GLfloat) * 4, 1, sizeof(GLfloat) * 8);
    program->enableAttributeArray(attr);

    attr = program->attributeLocation("mem");
    program->setAttributeBuffer(attr, GL_FLOAT, sizeof(GLfloat) * 5, 3, sizeof(GLfloat) * 8);
    program->enableAttributeArray(attr);



    VBO->release();

    glActiveTexture(GL_TEXTURE0_ARB);
    glBindTexture(target, tex_tar_id);
//    tex_tar.bind(0);
    program->setUniformValue("tex_tar",0);

    glActiveTexture(GL_TEXTURE1_ARB);
    glBindTexture(target, tex_src_id);
//    tex_src.bind(1);
    program->setUniformValue("tex_src",1);

    glActiveTexture(GL_TEXTURE2_ARB);
    glBindTexture(target, tex_weight_id);
    program->setUniformValue("tex_weight",2);

    glActiveTexture(GL_TEXTURE3_ARB);
    glBindTexture(target, tex_boundarycoords_id);
    program->setUniformValue("tex_boundarycoords",3);

    glActiveTexture(GL_TEXTURE4_ARB);
    glBindTexture(target, tmptex);
    program->setUniformValue("tex_err",4);


    program->setUniformValue("fx",(float)(fx/target_w));
    program->setUniformValue("fy",(float)(fy/target_h));
    program->setUniformValue("fx1",(int)fx);
    program->setUniformValue("fy1",(int)fy);
    program->setUniformValue("target_w",(float)target_w);
    program->setUniformValue("target_h",(float)target_h);
    program->setUniformValue("source_w",(float)source_w);
    program->setUniformValue("source_h",(float)source_h);
    program->setUniformValue("boundarysize",(int)selection.boundarySize);
//    program->setUniformValue("numpoints",selection.numPoints);
    QOpenGLVertexArrayObject::Binder vaoBind1(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

//    glFlush();

    program->release();
}

void GLWidget::paintMesh(){

    if (!init)
        return;
    int ind1, ind2, ind3;
    Point v1, v2, v3;

    float fx=selection.tx+selection.dx;
    float fy=selection.ty+selection.dy;
    Point shift(fx,fy);
    for (int i=0; i<selection.numTriangles; i++){

        glBegin(GL_LINES);
        ind1 = selection.triangles[i].p1;
        ind2 = selection.triangles[i].p2;
        ind3 = selection.triangles[i].p3;

        v1 = selection.vertices[ind1] + shift;
        v2 = selection.vertices[ind2] + shift;
        v3 = selection.vertices[ind3] + shift;

        glVertex2f(v1.x, v1.y);
        glVertex2f(v2.x, v2.y);
        glVertex2f(v2.x, v2.y);
        glVertex2f(v3.x, v3.y);
        glVertex2f(v3.x, v3.y);
        glVertex2f(v1.x, v1.y);

        glEnd();
    }
}

void GLWidget::resizeGL(int width, int height){

    /* Identity mapping between image and GL coordinates */
    glViewport(0, 0, (GLsizei)width, (GLsizei)height) ;
    glMatrixMode(GL_PROJECTION) ;
    glLoadIdentity() ;
    gluOrtho2D(0, (GLdouble)width, 0, (GLdouble)height) ;

}

void GLWidget::mousePressEvent(QMouseEvent *event){

    if (event->buttons() & Qt::LeftButton) {
        click_x = event->pos().x();
        click_y = event->pos().y();
    }
    update();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event){

    if (event->button() == Qt::LeftButton) {

        click_x = 0;
        click_y = 0;

        selection.tx += selection.dx;
        selection.ty += selection.dy;

        selection.dx = 0;
        selection.dy = 0;
    }
    update();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event){

    if (event->buttons() & Qt::LeftButton) {
        selection.dx = event->pos().x()-click_x;
        selection.dy = click_y-event->pos().y();
    }
    update();
}
