#makefile�ļ�����ָ��
#���make�ļ�����makefile��ֱ��ʹ��make�Ϳ��Ա���
#���make�ļ�������makefile������test.txt����ôʹ��make -f test.txt

#������������������������������ģ�顪����������������������#
#����һ��(make -C subdir) 
#��������(cd subdir && make)
#��-w ���Բ鿴ִ��ָ��ʱ����ǰ����Ŀ¼��Ϣ

#����������������������������Exist������������������������#
Output_lib_dir=./lib
Output_bin_dir=./bin
$(shell mkdir $(Output_lib_dir))
$(shell mkdir $(Output_bin_dir))

MDK_dir=./Micro-Development-Kit
Mother_Dir=./Mother
SSD_dir=./SolidStateDrive
Exist_dir=./Exist
CPU_dir=./CPU
Client_dir=./Client

all: 
	@echo "Create bin dir"
	tar -zxvpf bin.tar.gz
	rm -f bin.tar.gz
	@echo "Complie mdk"
	(make -C $(MDK_dir)/mdk_static -w)
	cp $(MDK_dir)/lib/mdk.a $(Output_lib_dir)/mdk.a
	@echo ""
	@echo "mdk Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie Mother"
	(make -C $(Mother_Dir) -w)
	@echo ""
	@echo "Mother Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie SolidStateDrive"
	(make -C $(SSD_dir) -w)
	@echo ""
	@echo "SolidStateDrive Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie Exist"
	(make -C $(Exist_dir) -w)
	@echo ""
	@echo "Exist Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie CPU"
	(make -C $(CPU_dir) -w)
	@echo ""
	@echo "CPU Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie Client"
	(make -C $(Client_dir) -w)
	@echo ""
	@echo "Client Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""


clean:
	rm $(Output_lib_dir)/*.a
	(make -C $(MDK_dir)/mdk_static -w clean)
	(make -C $(Mother_Dir) -w clean)
	(make -C $(SSD_dir) -w clean)
	(make -C $(Exist_dir) -w clean)
	(make -C $(Client_dir) -w clean)
