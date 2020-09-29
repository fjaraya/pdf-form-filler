#include "formfiller.h"
#include "maintask.h"

#include <QtDebug>
#include <QCoreApplication>

#include <poppler-form.h>

FormFiller::FormFiller(QString fileName, QObject *parent) :
    QObject(parent),
    fileName(fileName),
    document(Poppler::Document::load(fileName))
{
    if (document == NULL) {
        MainTask::fail("Document file could not be open");
    }
}

QList<FieldInfo *> FormFiller::listFieldsInfo()
{
    QList<FieldInfo *> output;
    int n = document->numPages();

    for (int i = 0; i < n; i += 1) {
        Poppler::Page *page = document->page(i);

        foreach (Poppler::FormField *field, page->formFields()) {
            if (!field->isReadOnly() && field->isVisible()) {
                if (field->type() == Poppler::FormField::FormText) {
                    output << new FieldInfo(
                                  i,
                                  "text",
                                  field->fullyQualifiedName(),
                                  ((Poppler::FormFieldText *)field)->text()
                              );
                }
            }
        }
    }

    return output;
}

void FormFiller::fill(const QMap<QString, FieldInfo *> &fieldsInfo)
{
    int n = document->numPages();

    for (int i = 0; i < n; i += 1) {
        Poppler::Page *page = document->page(i);

        foreach(Poppler::FormField *field, page->formFields()) {
            QString name = field->fullyQualifiedName();

            if (!field->isReadOnly()
                    && field->isVisible()
                    && fieldsInfo.contains(name)) {
                FieldInfo *info = fieldsInfo[name];

                if (field->type() == Poppler::FormField::FormText
                        && info->getType() == "text") {
                    Poppler::FormFieldText *textField = (Poppler::FormFieldText *) field;
                    textField->setText(info->getValue().toString());
                }
            }
        }
    }
}

void FormFiller::save(const QString &fileName)
{
    Poppler::PDFConverter *converter = document->pdfConverter();
    converter->setOutputFileName(fileName);
    converter->setPDFOptions(converter->pdfOptions() | Poppler::PDFConverter::WithChanges);

    if (!converter->convert()) {
        MainTask::fail("Saving output failed");
    }
}
