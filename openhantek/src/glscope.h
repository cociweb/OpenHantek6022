// SPDX-License-Identifier: GPL-2.0+

#pragma once

#include <list>
#include <memory>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QtGlobal>

#include "glscopegraph.h"
#include "hantekdso/enums.h"
#include "hantekprotocol/types.h"

struct DsoSettingsView;
struct DsoSettingsScope;
struct DsoSettingsScopeCursor;
class PPresult;

#define GLES100 "1.00 ES"
#define GLSL120 "1.20"
#define GLSL150 "1.50"

/// \brief OpenGL accelerated widget that displays the oscilloscope screen.
class GlScope : public QOpenGLWidget {
    Q_OBJECT

  public:
    static GlScope *createNormal( DsoSettingsScope *scope, DsoSettingsView *view, QWidget *parent = nullptr );
    static GlScope *createZoomed( DsoSettingsScope *scope, DsoSettingsView *view, QWidget *parent = nullptr );

    static void useOpenGLSLversion( QString version = GLSL120 );
    static QString getOpenGLversion();
    static QString getGLSLversion() { return GLSLversion; }
    /**
     * Show new post processed data
     * @param data
     */
    void showData( std::shared_ptr< PPresult > newData );
    void updateCursor( unsigned index = 0 );
    void cursorSelected( unsigned index ) {
        selectedCursor = index;
        updateCursor( index );
    }
    void generateGrid( int index = -1, double value = 0.0, bool pressed = false );

  protected:
    /// \brief Initializes the scope widget.
    /// \param settings The settings that should be used.
    /// \param parent The parent widget.
    GlScope( DsoSettingsScope *scope, DsoSettingsView *view, QWidget *parent = nullptr );
    ~GlScope() override;
    GlScope( const GlScope & ) = delete;

    /// \brief Initializes OpenGL output.
    void initializeGL() override;

    /// \brief Draw the graphs, marker and the grid.
    void paintGL() override;

    /// \brief Resize the widget.
    /// \param width The new width of the widget.
    /// \param height The new height of the widget.
    void resizeGL( int width, int height ) override;

    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseDoubleClickEvent( QMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void paintEvent( QPaintEvent *event ) override;

    /// \brief Draw the grid.
    void drawGrid();
    /// Draw vertical lines at marker positions
    void drawMarkers();
    void generateVertices( unsigned marker, const DsoSettingsScopeCursor &cursor );
    void drawVertices( QOpenGLFunctions *gl, unsigned marker, QColor color );

    void drawVoltageChannelGraph( ChannelID channel, Graph &graph, int historyIndex );
    void drawHistogramChannelGraph( ChannelID channel, Graph &graph, int historyIndex );
    void drawSpectrumChannelGraph( ChannelID channel, Graph &graph, int historyIndex );
    QPointF posToPosition( QPointF pos );
    void rightMouseEvent( QMouseEvent *event );

  signals:
    void markerMoved( unsigned cursorIndex, unsigned marker );
    void cursorMeasurement( QPointF position, bool status = true );

  private:
    // User settings
    DsoSettingsScope *scope;
    DsoSettingsView *view;
    bool zoomed = false;

    // Marker
    const unsigned NO_MARKER = UINT_MAX;
#pragma pack( push, 1 )
    struct Vertices {
        QVector3D a, b, c, d;
    };
#pragma pack( pop )
    const unsigned VERTICES_ARRAY_SIZE = sizeof( Vertices ) / sizeof( QVector3D );
    std::vector< Vertices > vaMarker;
    unsigned selectedMarker = NO_MARKER;
    QOpenGLBuffer m_marker;
    QOpenGLVertexArrayObject m_vaoMarker;

    // Cursors
    std::vector< DsoSettingsScopeCursor * > cursorInfo;
    unsigned selectedCursor = 0;
    bool rightMouseInside = false;
    QPointF rightMousePosition = QPointF();

    // Grid
    QOpenGLBuffer m_grid;
    static const int gridItems = 4;
    QOpenGLVertexArrayObject m_vaoGrid[ gridItems ];
    GLsizei gridDrawCounts[ gridItems ];
    void draw4Cross( std::vector< QVector3D > &va, int section, float x, float y );
    QColor triggerLineColor = QColor( "black" );

    // Graphs
    std::list< Graph > m_GraphHistory;
    unsigned currentGraphInHistory = 0;

    // OpenGL shader, matrix, var-locations
    static QString OpenGLversion;
    static QString GLSLversion;
    QString renderInfo;
    bool shaderCompileSuccess = false;
    QString errorMessage;
    std::unique_ptr< QOpenGLShaderProgram > m_program;
    QMatrix4x4 pmvMatrix; ///< projection, view matrix
    int colorLocation;
    int vertexLocation;
    int matrixLocation;
    int selectionLocation;
};
