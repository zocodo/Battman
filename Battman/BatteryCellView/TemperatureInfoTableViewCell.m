#import "TemperatureInfoTableViewCell.h"
#import "GradientArcView.h"

@implementation TemperatureCellView

- (instancetype)initWithFrame:(CGRect)frame percentage:(CGFloat)percentage {
    self = [super initWithFrame:frame];
    UIView *temperatureView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];
    temperatureView.layer.cornerRadius = 30;
    temperatureView.layer.masksToBounds = YES;
    [temperatureView.layer setBorderColor:[UIColor systemGray2Color].CGColor];
    [temperatureView.layer setBorderWidth:3];
    [temperatureView setBackgroundColor:[UIColor blackColor]];
//    temperatureView.layer.backgroundColor = CGColorCreateGenericRGB(0.15, 0.15, 0.15, 1.0);

    {
        GradientArcView *arcView = [[GradientArcView alloc] initWithFrame:temperatureView.bounds];
        arcView.center = temperatureView.center;
        [temperatureView addSubview:arcView];

        [arcView rotatePointerToPercent:percentage];
    }
    [self addSubview:temperatureView];
    return self;
}

@end

@implementation TemperatureInfoTableViewCell

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    TemperatureCellView *temperatureCell =
        [[TemperatureCellView alloc] initWithFrame:CGRectMake(20, 20, 80, 80) percentage:0.2];
    temperatureCell.translatesAutoresizingMaskIntoConstraints = NO;
    [self.contentView addSubview:temperatureCell];

    /*
    UILabel *temperatureLabel = [[UILabel alloc] initWithFrame:CGRectMake(120, 10, 600, 100)];
    temperatureLabel.lineBreakMode = NSLineBreakByWordWrapping;
    temperatureLabel.numberOfLines = 0;
    temperatureLabel.text = @"CPU温度：11.45℃\n电池温度：14.19℃\n充电器温度：19.81℃";
    [self.contentView addSubview:temperatureLabel];
     */

    //UICollectionView *collection = [[UICollectionView alloc] initWithFrame:CGRectZero];
    
    
    //_temperatureLabel = temperatureLabel;
    _temperatureCell = temperatureCell;

    return self;
}

@end
