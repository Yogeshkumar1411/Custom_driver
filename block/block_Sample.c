#include<linux/module.h>
#include<linux/fs.h>
#include<linux/genhd.h>
#include<linux/blkdev.h>


#define MY_BLOCK_MAJOR 240
#define MY_BLKDEV_NAME "mydev"
#define MY_BLOCK_MINORS 1
#define NR_SECTORS 1024
#define KERNEL_SECTOR_SIZE 512

static struct my_block_dev{

	spinlock_t lock;/*For mutual exclusion*/
	struct blk_mq_tag_set tag_set;
	struct request_queue *queue;
	struct gendisk *gd;


}dev;

struct bio{

	struct gendisk *bi_disk;
	unsigned int bi_opf;
	struct bio_vec *bi_io_vec;
	struct bvec_iter bi_iter;
	void *bi_private;
};

static blk_status_t my_block_request(struct blk_mq_hw_ctx *hctx,const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	struct my_block_dev *dev = q->queuedata;

	blk_mq_start_request(rq);

	if(blk_rq_is_passthrough(rq))
	{
		pr_info("Skip non-fs request\n");
		blk_mq_end_request(rq, BLK_STS_IOERR);
		goto out;
	}

	blk_mq_end_request(rq,BLK_STS_OK);

out:
	return BLK_STS_OK;
}
static struct blk_mq_ops my_queue_ops = {
	.queue_rq = my_block_request,
};


static int my_block_open(struct block_device *bdev,fmode_t mode)
{

	return 0;
}
static int my_block_release(struct gendisk *gd,fmode_t mode)
{

	return 0;
}


struct block_device_operations my_block_ops = {
	.owner = THIS_MODULE,
	.open = my_block_open,
	.release = my_block_release
};


static int create_block_device(struct my_block_dev *dev)
{
	dev->gd = alloc_disk(MY_BLOCK_MINORS);
	if(!dev->gd){
		pr_err("Alloc_disk_failure\n");
		return -ENOMEM;
	}


        dev->tag_set.ops = &my_queue_ops;
        dev->tag_set.nr_hw_queues = 1;
        dev->tag_set.queue_depth = 128;
        dev->tag_set.numa_mode = NUMA_NO_NODE;
        dev->tag_set.cmd_size = 0;
        dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
        err = blk_mq_alloc_tag_set(&dev->tag_set);
        if(err){
                goto out_err;
        }

        /*Allocate queue*/
        dev->queue = blk_mq_init_queue(&dev->tag_set);
        if(IS_ERR(dev->queue)){
                goto out_blk_init;
        }

        blk_queue_logical_block_size(dev->queue, KERNEL_SECTOR_SIZE);

        /*Assign private data to queue structure*/
        dev->queue->queuedata = dev;




	dev->gd->major       = MY_BLOCK_MAJOR;
	dev->gd->first_minor = 0;
	dev->gd->fops        = &my_block_ops;
	dev->gd->queue       = dev->queue;
	dev->gd->private_data = dev;
	snprintf(dev->gd->disk_name, 32, "myblock");
	set_capacity(dev->gd,NR_SECTORS);

	add_disk(dev->gd);

	return 0;

out_blk_init:
	blk_mq_free_tag_set(&dev->tag_set);
out_err:
	return -ENOMEM;
}

static int __init my_block_init(void){
	int status;
	status = register_blkdev(MY_BLOCK_MAJOR,MY_BLKDEV_NAME);
	if(status < 0)
	{
		pr_err("unable to register block device device\n");
		return -EBUSY;
	}
	status = create_block_device(&dev);
	if(status < 0)
	{
		return status;
	}
	
	return 0;
}
static void delete_block_device(struct my_block_dev *dev)
{
	if(dev->gd)
	{
		del_gendisk(dev->gd);
	}
	blk_mq_free_tag_set(&dev->tag_set);
	blk_cleanup_queue(dev->queue);
}


static void __exit my_block_exit(void){

	delete_block_device(&dev);	
	unregister_blkdev(MY_BLOCK_MAJOR,MY_BLKDEV_NAME);


}


module_init(my_block_init);
module_exit(my_block_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yogesh");
MODULE_DESCRIPTION("A sample block driver");

