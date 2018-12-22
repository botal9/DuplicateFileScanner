//
// Created by vitalya on 18.12.18.
//

#ifndef DUPLICATEFILESCANNER_TREEWIDGETITEM_H
#define DUPLICATEFILESCANNER_TREEWIDGETITEM_H


#include <QtWidgets/QTreeWidget>

constexpr const char* DUPLICATES_PREFIX = "Found ";
constexpr const char* DUPLICATES_SUFFIX = " duplicate(s)";

class TreeWidgetItem : public QTreeWidgetItem {
public:
    TreeWidgetItem();
    virtual ~TreeWidgetItem() = default;

    bool operator<(const QTreeWidgetItem& other) const override;

    void SetNumber(int column, int number);
};


#endif //DUPLICATEFILESCANNER_TREEWIDGETITEM_H
