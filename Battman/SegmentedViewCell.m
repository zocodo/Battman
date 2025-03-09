#import "SegmentedViewCell.h"

@implementation SegmentedViewCell

- (instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // This must exist for our custom labels to align
        self.textLabel.text = @"TITLE";
        // Hide
        self.textLabel.hidden = YES;
        self.accessoryView.hidden = YES;

        // Alternative title
        self.titleLabel = [[UILabel alloc] init];
        self.titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
        self.titleLabel.font = self.textLabel.font;
        [self.contentView addSubview:self.titleLabel];

        // Alternative detail
        self.detailLabel = [[UILabel alloc] init];
        self.detailLabel.translatesAutoresizingMaskIntoConstraints = NO;
        // For use as our custom labels' template
        UITableViewCell *cell;
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:nil];
        cell.hidden = YES;
        [self.contentView addSubview:cell];
        cell.detailTextLabel.text = @"DETAIL";
        self.detailLabel.font = cell.detailTextLabel.font;
        self.detailLabel.textColor = cell.detailTextLabel.textColor;
        self.detailLabel.textAlignment = cell.detailTextLabel.textAlignment;
        [self.contentView addSubview:self.detailLabel];
        
        // Initialize segmented control with sample segment
        self.segmentedControl = [[UISegmentedControl alloc] initWithItems:@[@"0"]];
        self.segmentedControl.translatesAutoresizingMaskIntoConstraints = NO;
        [self.contentView addSubview:self.segmentedControl];

        // Controlled content
        self.subTitleLabel = [[UILabel alloc] init];
        self.subTitleLabel.translatesAutoresizingMaskIntoConstraints = NO;
        self.subTitleLabel.font = self.textLabel.font;
        [self.contentView addSubview:self.subTitleLabel];
        self.subDetailLabel = [[UILabel alloc] init];
        self.subDetailLabel.translatesAutoresizingMaskIntoConstraints = NO;
        self.subDetailLabel.font = cell.detailTextLabel.font;
        self.subDetailLabel.textAlignment = cell.detailTextLabel.textAlignment;
        [self.contentView addSubview:self.subDetailLabel];

        self.subDetailLabel.text = @"0 mA";
        self.subTitleLabel.text = @"0 mV";
        
        // Setup Auto Layout constraints
        [NSLayoutConstraint activateConstraints:@[
            // Title label constraints
            [self.titleLabel.topAnchor constraintEqualToAnchor:self.textLabel.topAnchor constant:(self.frame.size.height - self.textLabel.font.pointSize) / 2],
            [self.titleLabel.leadingAnchor constraintEqualToAnchor:self.textLabel.leadingAnchor],
            [self.titleLabel.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor constant:-16],
            // Detail label
            [self.detailLabel.topAnchor constraintEqualToAnchor:self.textLabel.topAnchor constant:(self.frame.size.height - self.textLabel.font.pointSize) / 2],
            [self.detailLabel.leadingAnchor constraintEqualToAnchor:self.textLabel.leadingAnchor],
            [self.detailLabel.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor constant:-16],
            
            // Segmented control constraints
            [self.segmentedControl.topAnchor constraintEqualToAnchor:self.titleLabel.bottomAnchor constant:8],
            [self.segmentedControl.leadingAnchor constraintEqualToAnchor:self.contentView.leadingAnchor constant:16],
            [self.segmentedControl.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor constant:-16],
            //[self.segmentedControl.bottomAnchor constraintEqualToAnchor:self.contentView.bottomAnchor constant:-8],
            
            // Controlled text
            [self.subTitleLabel.topAnchor constraintEqualToAnchor:self.segmentedControl.bottomAnchor constant:8],
            [self.subTitleLabel.leadingAnchor constraintEqualToAnchor:self.textLabel.leadingAnchor],
            [self.subTitleLabel.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor constant:-16],
            [self.subDetailLabel.topAnchor constraintEqualToAnchor:self.segmentedControl.bottomAnchor constant:8],
            [self.subDetailLabel.leadingAnchor constraintEqualToAnchor:self.textLabel.leadingAnchor],
            [self.subDetailLabel.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor constant:-16],
            // Bottom
            [self.subDetailLabel.bottomAnchor constraintEqualToAnchor:self.contentView.bottomAnchor constant:-12],
        ]];
    }
    return self;
}

@end
