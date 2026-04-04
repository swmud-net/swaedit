#include "gui/NewExitWidget.h"
#include "ui_NewExitWidget.h"

#include <QGridLayout>

NewExitWidget::NewExitWidget(Room *room,
                             const QList<ExitDef> &exits,
                             int gridColumns,
                             Area *area,
                             const QMap<int, ExitDef> &exitsMap,
                             QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NewExitWidget)
    , room_(room)
    , exits_(exits)
    , area_(area)
    , exitsMap_(exitsMap)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    // Create direction button grid inside directionBox
    QGridLayout *grid = new QGridLayout();
    ui->directionBox->setLayout(grid);

    const int GRID_COLUMNS = gridColumns;
    int row = 0;
    int col = 0;

    for (const ExitDef &exitDef : exits_) {
        if (exitDef.empty) {
            // Empty placeholder -- advance grid position
            col++;
            if (col >= GRID_COLUMNS) {
                col = 0;
                row++;
            }
            continue;
        }

        QToolButton *btn = new QToolButton();
        btn->setText(exitDef.abbreviation);
        btn->setToolTip(exitDef.name);
        btn->setCheckable(true);
        btn->setMinimumSize(25, 25);

        // Disable if direction already used (except "somewhere" which can have multiples)
        bool alreadyUsed = roomHasExitInDirection(room_, exitDef.value);
        if (alreadyUsed && exitDef.name != "somewhere") {
            btn->setEnabled(false);
        }

        buttonDirectionMap_[btn] = exitDef.value;

        connect(btn, &QToolButton::clicked, this, &NewExitWidget::onDirectionButtonClicked);

        grid->addWidget(btn, row, col);

        col++;
        if (col >= GRID_COLUMNS) {
            col = 0;
            row++;
        }
    }

    // Set up radio buttons
    ui->twoWayButton->setChecked(true);
    connect(ui->oneWayButton, &QRadioButton::toggled, this, &NewExitWidget::onOneWayToggled);
    connect(ui->twoWayButton, &QRadioButton::toggled, this, &NewExitWidget::onTwoWayToggled);

    connect(ui->acceptButton, &QPushButton::clicked, this, &NewExitWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &NewExitWidget::onCancelClicked);

    // Initially disabled until direction is selected
    ui->acceptButton->setEnabled(false);
    ui->destinationVnumBox->setEnabled(false);

    fillDestinationBox();
}

NewExitWidget::~NewExitWidget()
{
    delete ui;
}

void NewExitWidget::fillDestinationBox()
{
    ui->destinationVnumBox->clear();
    ui->destinationVnumBox->setEnabled(true);

    bool twoWay = ui->twoWayButton->isChecked();

    for (const Room &r : area_->rooms) {
        if (twoWay && selectedDirection_ >= 0) {
            // For two-way, exclude rooms that already have the reverse exit pointing back
            int oppositeDir = -1;
            if (exitsMap_.contains(selectedDirection_))
                oppositeDir = exitsMap_[selectedDirection_].opposite;

            if (oppositeDir >= 0) {
                bool hasReverse = false;
                for (const Exit &e : r.exits) {
                    if (e.direction == oppositeDir) {
                        hasReverse = true;
                        break;
                    }
                }
                if (hasReverse) continue;
            }
        }

        ui->destinationVnumBox->addItem(
            QString::number(r.vnum) + " - " + r.name,
            QVariant(r.vnum));
    }
}

bool NewExitWidget::roomHasExitInDirection(const Room *room, int direction) const
{
    if (!room) return false;
    for (const Exit &e : room->exits) {
        if (e.direction == direction)
            return true;
    }
    return false;
}

void NewExitWidget::onDirectionButtonClicked()
{
    QToolButton *btn = qobject_cast<QToolButton *>(sender());
    if (!btn) return;

    // Uncheck previous selection
    if (selectedButton_ && selectedButton_ != btn) {
        selectedButton_->setChecked(false);
    }

    if (btn->isChecked()) {
        selectedButton_ = btn;
        selectedDirection_ = buttonDirectionMap_[btn];
        ui->oneWayButton->setEnabled(false);
        ui->twoWayButton->setEnabled(false);
    } else {
        selectedButton_ = nullptr;
        selectedDirection_ = -1;
        ui->oneWayButton->setEnabled(true);
        ui->twoWayButton->setEnabled(true);
    }

    // Refill destination box for two-way filtering
    fillDestinationBox();

    // Enable accept only after destination list is rebuilt
    ui->acceptButton->setEnabled(selectedDirection_ >= 0 &&
                                  ui->destinationVnumBox->count() > 0);
}

void NewExitWidget::onOneWayToggled(bool checked)
{
    if (checked) {
        fillDestinationBox();
    }
}

void NewExitWidget::onTwoWayToggled(bool checked)
{
    if (checked) {
        fillDestinationBox();
    }
}

void NewExitWidget::onAcceptClicked()
{
    if (selectedDirection_ < 0) return;
    if (ui->destinationVnumBox->currentIndex() < 0) return;

    qint64 destVnum = ui->destinationVnumBox->currentData().toLongLong();

    // Create the exit
    Exit newExit;
    newExit.direction = selectedDirection_;
    newExit.vnum = destVnum;
    newExit.distance = 0;
    newExit.flags = 0;
    newExit.key = -1;
    room_->exits.append(newExit);

    // If two-way, create reverse exit in destination room
    if (ui->twoWayButton->isChecked()) {
        int oppositeDir = -1;
        if (exitsMap_.contains(selectedDirection_))
            oppositeDir = exitsMap_[selectedDirection_].opposite;

        // For "somewhere" (oppositeDir < 0), use same direction as forward exit
        int revDir = (oppositeDir >= 0) ? oppositeDir : selectedDirection_;

        for (Room &r : area_->rooms) {
            if (r.vnum == destVnum) {
                Exit reverseExit;
                reverseExit.direction = revDir;
                reverseExit.vnum = room_->vnum;
                reverseExit.distance = 0;
                reverseExit.flags = 0;
                reverseExit.key = -1;
                r.exits.append(reverseExit);
                break;
            }
        }
    }

    emit exitCreated();
    close();
}

void NewExitWidget::onCancelClicked()
{
    close();
}
