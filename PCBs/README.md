IRD PCB v4.1
----

This PCB for the Arduino ProMini (3.3v, 8MHz version) can be used for prototyping and even integration purpose. It allows much simpler wiring of the soil humidity sensor, the watermark sensors and the soil temperature sensor (placeholder for associated resistors are already there).

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-4-1-raw-top.png" width="400">

This new PCB also integrates by default a low-cost solar charging circuit to add solar panel and rechargeable NiMh batteries. The whole solar circuit appears on the back of the PCB (left part of the PCB) as illustrated below.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-4-1-raw-bottom.png" width="400">

You can download the Gerber zipped archive and view them on an [online Gerber viewer](https://www.pcbgogo.com/GerberViewer.html).

- Arduino ProMini IRD PCB v4.1 for RFM95W/RFM96W zipped Gerber archive, 2 layer board of 30.89x79.5mm [.zip](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_4_1/Gerber_PCB4_1_IISS_2023_09_27.zip)

The raw PCB will of course not have the additional electronic components. While it is possible to add these component manually, in practice it is not recommended as the components are small and manually soldering them is prone to errors. 

IRD PCBA v4.1
----

Instead, the fully assembled PCB (PCBA) with solar circuit can be ordered fully assembled from PCB manufacturers.

You can download the BOM & CPL files to order the fully assembled board from PCB manufacturers. You can look at the tutorial produced in the context of the INTEL-IRRIS project on how to order PCB that are fully assembled by the manufacturer: [Building the INTEL-IRRIS IoT platform. Annex 1: ordering the PCBs, including PCBA](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-PCB-update-PCBA.pdf).  

- Bill of Materiels (BOM) file for the IRD PCB v4.1 RFM95W 868 [.xls](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_4_1/BOM_SMT_TB_RFM95_868_IISS_PCB4_1_wH2.xlsx)

- CPL file for the IRD PCB v4.1 [.csv](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_4_1/CPL.csv)

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-4-1-3D-top.png" width="400">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-4-1-3D-bottom.png" width="400">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-4-1-real-top-removebg-preview.png" width="400">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-4-1-real-bottom-removebg-preview.png" width="400">

Read [INTEL-IRRIS Newsletter #4](https://intel-irris.eu/intel-irris-newsletter-4) that presented a brief description with additional images.

IRD PCB v5
----

This PCB is the RAK3172 version of the IRD PCB v4.1.

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-5-raw-top.png" width="400">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/ird-pcb-5-raw-bottom.png" width="400">

You can download the Gerber zipped archive and view them on an [online Gerber viewer](https://www.pcbgogo.com/GerberViewer.html).

- Arduino ProMini IRD PCB v5 for RAK3172 zipped Gerber archive, 2 layer board of 30.89x79.5mm [.zip](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_5/Gerber_PCB5_RAK3172_IISS_2024-06-01.zip)

Because the RAK3172 module is connected to the Arduino's serial port (UART), there is a specific procedure to debug the board in order to see the text output normally printed to the serial monitor window of the Arduino IDE. See the [tutorial on using this IRD PCB v5](https://docs.google.com/viewer?url=https://github.com/CongducPham/PRIMA-Intel-IrriS/raw/main/Tutorials/Intel-Irris-IOT-platform-PCBv5-PCBA.pdf) produced in the context of the INTEL-IRRIS project. A correct output would be similar to [this output example](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_5/dump-serial-debug.txt)

IRD PCBA v5
----

The fully assembled PCB (PCBA) with solar circuit can be ordered fully assembled from PCB manufacturers.

You can download the BOM & CPL files to order the fully assembled board from PCB manufacturers. You can look at the tutorial produced in the context of the INTEL-IRRIS project on how to order PCB that are fully assembled by the manufacturer: [Building the INTEL-IRRIS IoT platform. Annex 1: ordering the PCBs, including PCBA](https://github.com/CongducPham/PRIMA-Intel-IrriS/blob/main/Tutorials/Intel-Irris-PCB-update-PCBA.pdf). 

- Bill of Materiels (BOM) file for the IRD PCB v5 RAK3172 [.xls](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_5/BOM_SMT_TB_RAK3172_868_IISS_PCB5)

- CPL file for the IRD PCB v5 [.csv](https://github.com/CongducPham/PEPR_AgriFutur/raw/main/PCBs/IRD_PCB_5/CPL_PCB5_RAK3172.csv)

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/PCBA_v5_top.png" width="400">

<img src="https://github.com/CongducPham/PEPR_AgriFutur/blob/main/images/PCBA_v5_bottom.png" width="400">




