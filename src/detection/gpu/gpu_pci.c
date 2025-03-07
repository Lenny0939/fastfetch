#include "gpu.h"

void ffGPUParsePciIds(FFstrbuf* content, uint8_t subclass, uint16_t vendor, uint16_t device, uint16_t subVendor, uint16_t subDevice, FFGPUResult* gpu)
{
    if (content->length)
    {
        char buffer[32];

        // Search for vendor
        uint32_t len = (uint32_t) snprintf(buffer, sizeof(buffer), "\n%04x  ", vendor);
        char* start = (char*) memmem(content->chars, content->length, buffer, len);
        char* end = content->chars + content->length;
        if (start)
        {
            start += len;
            end = memchr(start, '\n', (uint32_t) (end - start));
            if (!end)
                end = content->chars + content->length;
            if (!gpu->vendor.length)
                ffStrbufSetNS(&gpu->vendor, (uint32_t) (end - start), start);

            start = end; // point to '\n' of vendor
            end = start + 1; // point to start of devices
            // find the start of next vendor
            while (end[0] == '\t' || end[0] == '#')
            {
                end = strchr(end, '\n');
                if (!end)
                {
                    end = content->chars + content->length;
                    break;
                }
                else
                    end++;
            }

            // Search for device
            len = (uint32_t) snprintf(buffer, sizeof(buffer), "\n\t%04x  ", device);
            start = memmem(start, (size_t) (end - start), buffer, len);
            if (start)
            {
                start += len;

                // Search for subvendor and subdevice
                len = (uint32_t) snprintf(buffer, sizeof(buffer), "\n\t\t%04x %04x  ", subVendor, subDevice);
                char* subStart = memmem(start, (size_t) (end - start), buffer, len);
                if (subStart)
                    start = subStart + len;

                end = memchr(start, '\n', (uint32_t) (end - start));
                if (!end)
                    end = content->chars + content->length;

                char* openingBracket = memchr(start, '[', (uint32_t) (end - start));
                if (openingBracket)
                {
                    openingBracket++;
                    char* closingBracket = memchr(openingBracket, ']', (uint32_t) (end - openingBracket));
                    if (closingBracket)
                        ffStrbufSetNS(&gpu->name, (uint32_t) (closingBracket - openingBracket), openingBracket);
                }
                if (!gpu->name.length)
                    ffStrbufSetNS(&gpu->name, (uint32_t) (end - start), start);
            }
        }
    }

    if (!gpu->name.length)
    {
        const char* subclassStr;
        switch (subclass)
        {
        case 0 /*PCI_CLASS_DISPLAY_VGA*/: subclassStr = " (VGA compatible)"; break;
        case 1 /*PCI_CLASS_DISPLAY_XGA*/: subclassStr = " (XGA compatible)"; break;
        case 2 /*PCI_CLASS_DISPLAY_3D*/: subclassStr = " (3D)"; break;
        default: subclassStr = ""; break;
        }

        ffStrbufSetF(&gpu->name, "%s Device %04X%s", gpu->vendor.length ? gpu->vendor.chars : "Unknown", device, subclassStr);
    }
}
