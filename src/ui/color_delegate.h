/*
 * Delegate to apply color scheme based on semantic roles
 * src/ui/color_delegate.h
 */

#ifndef PCMANFM_COLOR_DELEGATE_H
#define PCMANFM_COLOR_DELEGATE_H

#include <QStyledItemDelegate>

#include "color_manager.h"

namespace PCManFM {

class ColorDelegate : public QStyledItemDelegate {
    Q_OBJECT
   public:
    explicit ColorDelegate(ColorManager* colors, QObject* parent = nullptr);

   protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   private:
    ColorManager* colors_;
};

}  // namespace PCManFM

#endif  // PCMANFM_COLOR_DELEGATE_H
