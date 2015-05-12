#ifndef MATERIALLIBRARY_H
#define MATERIALLIBRARY_H

/// @class MaterialLibrary
/// @author Declan Russell
/// @date 05/03/15
/// @brief A wigdet to store and allow the user to select between
/// @brief optix materials availible in out program.
/// @brief This is a singleton class as we want all instances of our
/// @brief library to be the same.

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QSpacerItem>
#include <optixu/optixpp_namespace.h>
#include <map>
#include <QListWidget>
#include <QListWidgetItem>


class MaterialLibrary : public QWidget
{
    Q_OBJECT
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief get an instance of our material library
    //------------------------------------------------------------------------------------------------------------------------------------
    static MaterialLibrary *getInstance();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor. Deals with our garbage collection
    //------------------------------------------------------------------------------------------------------------------------------------
    ~MaterialLibrary();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief add a material to our library
    /// @param _name - the name of the material to add
    /// @param _material - the optix material to add to the library
    /// @return bool indicating if the material has been successfully added to the library
    //------------------------------------------------------------------------------------------------------------------------------------
    bool addMaterialToLibrary(std::string _name, optix::Material _material);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief removes our singleton class from existance
    //------------------------------------------------------------------------------------------------------------------------------------
    inline void destroy(){delete m_instance;}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief function let the user select a material from our library and apply it to a mesh
    /// @param _meshId - id of mesh to apply it to
    //------------------------------------------------------------------------------------------------------------------------------------
    void applyLibMatToMesh(std::string _meshId);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overide our close event to reset our variables
    //------------------------------------------------------------------------------------------------------------------------------------
    void closeEvent(QCloseEvent * event);
    //------------------------------------------------------------------------------------------------------------------------------------
signals:

public slots:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief slot to be called if something is selected in our material list
    /// @param _item - item selected in list
    //------------------------------------------------------------------------------------------------------------------------------------
    void matSelected(QListWidgetItem *_item);
    //------------------------------------------------------------------------------------------------------------------------------------
private:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief pointer to the instance of our singleton class
    //------------------------------------------------------------------------------------------------------------------------------------
    static MaterialLibrary* m_instance;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief defalut constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    explicit MaterialLibrary(QWidget *parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief the layout of our wigdet
    //------------------------------------------------------------------------------------------------------------------------------------
    QGridLayout *m_widgetLayout;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief spacer for our widget
    //------------------------------------------------------------------------------------------------------------------------------------
    QSpacerItem *m_widgetSpacer;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief map of our materials
    //------------------------------------------------------------------------------------------------------------------------------------
    std::map<std::string,optix::Material> m_materials;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief list widget to display the materials in our library
    //------------------------------------------------------------------------------------------------------------------------------------
    QListWidget *m_matListWidget;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief a bool to indicate if we are applying the next selected material to a mesh
    //------------------------------------------------------------------------------------------------------------------------------------
    bool m_applyMatToMesh;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief mesh to apply material to
    //------------------------------------------------------------------------------------------------------------------------------------
    std::string m_selectedMeshId;
    //------------------------------------------------------------------------------------------------------------------------------------
};

#endif // MATERIALLIBRARY_H
