//
// Created by vitalya on 18.12.18.
//

#include "TreeWidgetItem.h"

#include <QRegExp>

bool TreeWidgetItem::operator<(const QTreeWidgetItem& other) const {
    int lhs = 0;
    int rhs = 0;
    try {
        lhs = this->text(1).toInt();
    } catch (std::exception& e) {
        return false;
    }
    try {
        rhs = other.text(1).toInt();
    } catch (std::exception& e) {
        return false;
    }
    return lhs < rhs;
}

TreeWidgetItem::TreeWidgetItem()
    : QTreeWidgetItem()
{
}
