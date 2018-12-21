//
// Created by vitalya on 18.12.18.
//

#ifndef DUPLICATEFILESCANNER_TREEWIDGETITEM_H
#define DUPLICATEFILESCANNER_TREEWIDGETITEM_H


#include <QtWidgets/QTreeWidget>

class TreeWidgetItem : public QTreeWidgetItem {
public:
    TreeWidgetItem();
    virtual ~TreeWidgetItem() = default;

    bool operator<(const QTreeWidgetItem& other) const override;
};


#endif //DUPLICATEFILESCANNER_TREEWIDGETITEM_H
