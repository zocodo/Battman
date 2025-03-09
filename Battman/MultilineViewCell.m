#import "MultilineViewCell.h"

@implementation MultilineViewCell

- (instancetype)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Remove or ignore the built-in textLabel
        self.textLabel.hidden = YES;
        self.textLabel.text = @"Multiline Title";

        // Setup titleLabel
        self.titleLabel = [[UILabel alloc] init];
        self.titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
        self.titleLabel.font = self.textLabel.font;
        [self.contentView addSubview:self.titleLabel];

        // Setup detailLabel as a UILabel for simplicity
        self.detailLabel = [[UILabel alloc] init];
        self.detailLabel.translatesAutoresizingMaskIntoConstraints = NO;
        // For use as our custom labels' template
        UITableViewCell *cell;
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:nil];
        cell.hidden = YES;
        [self.contentView addSubview:cell];
        cell.detailTextLabel.text = @"DETAIL";
        self.detailLabel.numberOfLines = 0;
        self.detailLabel.font = cell.detailTextLabel.font;
        self.detailLabel.textColor = cell.detailTextLabel.textColor;
        self.detailLabel.textAlignment = cell.detailTextLabel.textAlignment;
        [self.contentView addSubview:self.detailLabel];

        // Setup Auto Layout constraints relative to contentView
        [NSLayoutConstraint activateConstraints:@[
            // Title label constraints
            [self.titleLabel.topAnchor constraintEqualToAnchor:self.textLabel.topAnchor constant:(self.frame.size.height - self.textLabel.font.pointSize) / 2],
            [self.titleLabel.leadingAnchor constraintEqualToAnchor:self.textLabel.leadingAnchor],
            [self.titleLabel.trailingAnchor constraintEqualToAnchor:self.textLabel.trailingAnchor],
            
            // Detail label constraints
            [self.detailLabel.topAnchor constraintEqualToAnchor:self.textLabel.topAnchor constant:(self.frame.size.height - self.textLabel.font.pointSize) / 2],
            [self.detailLabel.leadingAnchor constraintEqualToAnchor:self.textLabel.leadingAnchor],
            [self.detailLabel.trailingAnchor constraintEqualToAnchor:self.textLabel.trailingAnchor],
            [self.detailLabel.bottomAnchor constraintEqualToAnchor:self.contentView.bottomAnchor constant:-12],
        ]];
    }
    return self;
}

@end
