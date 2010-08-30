/*=========================================================================

  Library:   CTK
 
  Copyright (c) 2010  Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.commontk.org/LICENSE

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 
  =========================================================================*/

// Qt includes
#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QBoxLayout>
#include <QDebug>
#include <QPointer>
#include <QStyle>

// CTK includes
#include "ctkWorkflowButtonBoxWidget.h"
#include "ctkWorkflowStep.h"
#include "ctkWorkflowWidgetStep.h"
#include "ctkWorkflow.h"

// STD includes
#include <iostream>

//-----------------------------------------------------------------------------
class ctkWorkflowButtonBoxWidgetPrivate : public ctkPrivate<ctkWorkflowButtonBoxWidget>
{
public:
  CTK_DECLARE_PUBLIC(ctkWorkflowButtonBoxWidget);
  ctkWorkflowButtonBoxWidgetPrivate();
  ~ctkWorkflowButtonBoxWidgetPrivate(){}

  void setupUi(QWidget * newParent);

  QPointer<ctkWorkflow>    Workflow;

  // Convenient typedefs
  typedef QMap<QPushButton*, ctkWorkflowStep*> ButtonToStepMapType;

  // The buttons on the widget (maintain maps associating each forward/goTo button with its step)
  QPushButton*        BackButton;
  QString             BackButtonDefaultText;
  QPushButton*        NextButton;
  QString             NextButtonDefaultText;
  ButtonToStepMapType GoToButtonToStepMap;

  // Direction for layout (for use with QBoxLayout only)
  QBoxLayout::Direction Direction;

  bool HideInvalidButtons;

  // Functions to add/remove buttons
  void updateBackButton(ctkWorkflowStep* currentStep);
  void updateNextButton(ctkWorkflowStep* currentStep);
  void updateGoToButtons(ctkWorkflowStep* currentStep);

};

//-----------------------------------------------------------------------------
// ctkWorkflowButtonBoxWidgetPrivate methods

//-----------------------------------------------------------------------------
ctkWorkflowButtonBoxWidgetPrivate::ctkWorkflowButtonBoxWidgetPrivate()
{
  this->BackButton = 0;
  this->BackButtonDefaultText = "Back";
  this->NextButton = 0;
  this->NextButtonDefaultText = "Next";
  this->Direction = QBoxLayout::LeftToRight;
  this->HideInvalidButtons = false;
}

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidgetPrivate::setupUi(QWidget * newParent)
{
  QBoxLayout * boxLayout = new QBoxLayout(this->Direction, newParent);
  boxLayout->setSpacing(0);
  boxLayout->setContentsMargins(0, 0, 0, 0);
  newParent->setLayout(boxLayout);

  // Retrieve standard icons
  QIcon backIcon = newParent->style()->standardIcon(QStyle::SP_ArrowLeft);
  QIcon nextIcon = newParent->style()->standardIcon(QStyle::SP_ArrowRight);

  // Setup the buttons
  this->BackButton = new QPushButton(backIcon, this->BackButtonDefaultText, newParent);
  newParent->layout()->addWidget(this->BackButton);

  this->NextButton = new QPushButton(nextIcon, this->NextButtonDefaultText, newParent);
  this->NextButton->setLayoutDirection(Qt::RightToLeft);
  newParent->layout()->addWidget(this->NextButton);
}

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidgetPrivate::updateBackButton(ctkWorkflowStep* currentStep)
{
  CTK_P(ctkWorkflowButtonBoxWidget);

  Q_ASSERT(this->Workflow);
  Q_ASSERT(currentStep);
  Q_ASSERT(p->layout());

  ctkWorkflowWidgetStep* step = qobject_cast<ctkWorkflowWidgetStep*>(currentStep);

  // Set the back button text
  QString backButtonText = this->BackButtonDefaultText;
  if (step && !step->backButtonText().isEmpty())
    {
    backButtonText = step->backButtonText();
    }
  this->BackButton->setText(backButtonText);

  // Enable and show the back button if we can go backward
  if (this->Workflow->canGoBackward(currentStep))
    {
    this->BackButton->setEnabled(true);
    this->BackButton->show();

    // Apply the buttonBox hints if possible
    if (step)
      {
      this->BackButton->setDisabled(
          step->buttonBoxHints() & ctkWorkflowWidgetStep::BackButtonDisabled);
      this->BackButton->setHidden(
          step->buttonBoxHints() & ctkWorkflowWidgetStep::BackButtonHidden);
      }
    }
  // Disable the back button if we can't go backward, and optionally hide it
  else
    {
    this->BackButton->setEnabled(false);
    this->HideInvalidButtons ? this->BackButton->hide() : this->BackButton->show();
    }
}

//-----------------------------------------------------------------------------
// This will change for branching workflows, to look more like updateGoToButtons()
void ctkWorkflowButtonBoxWidgetPrivate::updateNextButton(ctkWorkflowStep* currentStep)
{
  CTK_P(ctkWorkflowButtonBoxWidget);

  Q_ASSERT(this->Workflow);
  Q_ASSERT(currentStep);
  Q_ASSERT(p->layout());

  ctkWorkflowWidgetStep* step = qobject_cast<ctkWorkflowWidgetStep*>(currentStep);

  // Set the next button text
  QString nextButtonText = this->NextButtonDefaultText;
  if (step && !step->nextButtonText().isEmpty())
    {
    nextButtonText = step->nextButtonText();
    }
  this->NextButton->setText(nextButtonText);

  // Enable and show the next button if we can go forward
  if (this->Workflow->canGoForward(currentStep))
    {
    this->NextButton->setEnabled(true);
    this->NextButton->show();

    // Apply the buttonBox hints if possible
    if (step)
      {
      this->NextButton->setDisabled(
          step->buttonBoxHints() & ctkWorkflowWidgetStep::NextButtonDisabled);
      this->NextButton->setHidden(
          step->buttonBoxHints() & ctkWorkflowWidgetStep::NextButtonHidden);
      }
    }
  // Disable the next button if we can't go forward, and optionally hide it
  else
    {
    this->NextButton->setEnabled(false);
    this->HideInvalidButtons ? this->NextButton->hide() : this->NextButton->show();
    }
}

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidgetPrivate::updateGoToButtons(ctkWorkflowStep* currentStep)
{
  CTK_P(ctkWorkflowButtonBoxWidget);

  Q_ASSERT(this->Workflow);
  Q_ASSERT(currentStep);
  Q_ASSERT(p->layout());

  // Change the buttons only if the set of steps to have goTo buttons is either empty or has changed
  QSet<ctkWorkflowStep*> goToStepsToHaveButtons = QSet<ctkWorkflowStep*>::fromList(this->Workflow->finishSteps());
  QSet<ctkWorkflowStep*> goToStepsThatHaveButtons = QSet<ctkWorkflowStep*>::fromList(this->GoToButtonToStepMap.values());

  // Remove the buttons if the set of steps to have goTo buttons has changed
  if (!this->GoToButtonToStepMap.isEmpty() && goToStepsThatHaveButtons != goToStepsToHaveButtons)
    {
    foreach (QPushButton* goToButton, this->GoToButtonToStepMap.keys())
      {
      p->layout()->removeWidget(goToButton);
      delete goToButton;
      }
    this->GoToButtonToStepMap.clear();
    }

  // Create the buttons, either for the first time or after removing them above
  if (this->GoToButtonToStepMap.isEmpty())
    {
    // Create a button for each of the goToSteps
    foreach (ctkWorkflowStep* step, goToStepsToHaveButtons)
      {
      // TODO shouldn't have id here
      QPushButton* goToButton = new QPushButton(step->id());
      p->layout()->addWidget(goToButton);
      QObject::connect(goToButton, SIGNAL(clicked()), p, SLOT(prepareGoToStep()));
      this->GoToButtonToStepMap[goToButton] = step;
      }
      
    }

  // Show/hide the goTo buttons depending on whether they are accessible from the current step
  foreach (QPushButton* goToButton, this->GoToButtonToStepMap.keys())
    {
    // TODO enable and show the goTo button if we can go to it
    // ctkWorkflowStep* goToStep = this->GoToButtonToStepMap[goToButton];
    // if (this->Workflow->canGoToStep(currentStep, goToStep))
    // for now we'll assume we can go to the step
    if (true)
      {
      goToButton->setEnabled(true);
      goToButton->show();
      }
    // disable the goTo button if we can't go to it, and optionally hide it
    else
      {
      goToButton->setEnabled(false);
      this->HideInvalidButtons ? goToButton->hide() : goToButton->show();
      }
    }
}

//-----------------------------------------------------------------------------
// ctkWorkflowButtonBoxWidget methods

//-----------------------------------------------------------------------------
ctkWorkflowButtonBoxWidget::ctkWorkflowButtonBoxWidget(ctkWorkflow* newWorkflow, QWidget* newParent) :
    Superclass(newParent)
{
  CTK_INIT_PRIVATE(ctkWorkflowButtonBoxWidget);
  CTK_D(ctkWorkflowButtonBoxWidget);

  d->setupUi(this);

  this->setWorkflow(newWorkflow);
}

//-----------------------------------------------------------------------------
ctkWorkflowButtonBoxWidget::ctkWorkflowButtonBoxWidget(QWidget* newParent) :
    Superclass(newParent)
{
  CTK_INIT_PRIVATE(ctkWorkflowButtonBoxWidget);
  CTK_D(ctkWorkflowButtonBoxWidget);

  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidget::setWorkflow(ctkWorkflow * newWorkflow)
{
  CTK_D(ctkWorkflowButtonBoxWidget);

  if (d->Workflow == newWorkflow)
    {
    return;
    }

  // Disconnect
  if (!d->Workflow.isNull())
    {
    QObject::disconnect(d->BackButton, SIGNAL(clicked()), d->Workflow, SLOT(goBackward()));
    QObject::disconnect(d->NextButton, SIGNAL(clicked()), d->Workflow, SLOT(goForward()));
    }

  // Connect
  if (newWorkflow)
    {
    QObject::connect(d->BackButton, SIGNAL(clicked()), newWorkflow, SLOT(goBackward()));
    QObject::connect(d->NextButton, SIGNAL(clicked()), newWorkflow, SLOT(goForward()));
    }

  d->Workflow = newWorkflow;
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(ctkWorkflowButtonBoxWidget, QString, backButtonDefaultText,
            BackButtonDefaultText);

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidget::setBackButtonDefaultText(const QString& defaultText)
{
  CTK_D(ctkWorkflowButtonBoxWidget);
  d->BackButtonDefaultText = defaultText;
  if (d->Workflow)
    {
    this->updateButtons();
    }
  else
    {
    d->BackButton->setText(d->BackButtonDefaultText);
    }
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(ctkWorkflowButtonBoxWidget, QString, nextButtonDefaultText,
            NextButtonDefaultText);

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidget::setNextButtonDefaultText(const QString& defaultText)
{
  CTK_D(ctkWorkflowButtonBoxWidget);
  d->NextButtonDefaultText = defaultText;
  if (d->Workflow)
    {
    this->updateButtons();
    }
  else
    {
    d->NextButton->setText(d->NextButtonDefaultText);
    }
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(ctkWorkflowButtonBoxWidget, ctkWorkflow*, workflow, Workflow);
CTK_GET_CXX(ctkWorkflowButtonBoxWidget, QPushButton*, backButton, BackButton);
CTK_GET_CXX(ctkWorkflowButtonBoxWidget, QBoxLayout::Direction, direction, Direction);
CTK_GET_CXX(ctkWorkflowButtonBoxWidget, bool, hideInvalidButtons, HideInvalidButtons);
CTK_SET_CXX(ctkWorkflowButtonBoxWidget, bool, setHideInvalidButtons, HideInvalidButtons);

//-----------------------------------------------------------------------------
// TODO will be list of next buttons for branching workflow
QPushButton* ctkWorkflowButtonBoxWidget::nextButton()const
{
  CTK_D(const ctkWorkflowButtonBoxWidget);
  return d->NextButton;
}

//-----------------------------------------------------------------------------
QList<QPushButton*> ctkWorkflowButtonBoxWidget::goToButtons()const
{
  CTK_D(const ctkWorkflowButtonBoxWidget);
  return d->GoToButtonToStepMap.keys();
}

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidget::setDirection(const QBoxLayout::Direction& newDirection)
{
  if (QBoxLayout* layout = qobject_cast<QBoxLayout*>(this->layout()))
    {
    layout->setDirection(newDirection);
    }
}

//-----------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidget::updateButtons()
{

  CTK_D(ctkWorkflowButtonBoxWidget);
  if (d->Workflow.isNull())
    {
    return;
    }

  ctkWorkflowStep* currentStep = d->Workflow->currentStep();
  if (!currentStep)
    {
    return;
    }

  d->updateBackButton(currentStep);
  d->updateNextButton(currentStep);
  d->updateGoToButtons(currentStep);
}

//--------------------------------------------------------------------------
void ctkWorkflowButtonBoxWidget::prepareGoToStep()
{
  CTK_D(ctkWorkflowButtonBoxWidget);
  if (QPushButton* button = qobject_cast<QPushButton*>(QObject::sender()))
    {
    if (ctkWorkflowStep* step = d->GoToButtonToStepMap.value(button))
      {
      d->Workflow->goToStep(step->id());
      }
    }
}