
#include "mrvFl/mrvIO.h"

#include "mrvPDF/mrvPDFCreator.h"
#include "mrvPDF/mrvSavePDF.h"

#include "mrViewer.h"

namespace
{
    const char* kModule = "pdf";
}

namespace mrv
{
    bool save_pdf(const std::string& file, const ViewerUI* ui)
    {
        auto player = ui->uiView->getTimelinePlayer();
        if (!player)
            return false;

        const auto& annotations = player->getAllAnnotations();
        if (annotations.empty())
            return false;

        std::string pdfFile = file;
        if (file.substr(file.size() - 4, file.size()) != ".pdf")
        {
            pdfFile += ".pdf";
        }

        PDFCreator pdf(pdfFile, annotations, ui);
        try
        {
            pdf.create();
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(e.what());
            return false;
        }
    }
} // namespace mrv
