#include "RectangleShape.hpp"

#include "Shape2DUtils.hpp"

#include <sstream>

namespace View
{
	/**
	 *
	 */
	RectangleShape::RectangleShape( const std::string& aTitle /*= ""*/) :
								centre( -1, -1),
								size( 0, 0),
								title( aTitle),
								titleSize( 0, 0),
								borderWidth( 2),
								spacing( 2)
	{
	}
	/**
	 *
	 */
	RectangleShape::RectangleShape( Model::ModelObjectPtr aModelObject,
									const wxPoint& aCentrePoint,
									const std::string& aTitle /*= ""*/,
									int aBorderWidth /*= 2*/,
									int aSpacing /*= 2*/) :
								Shape(aModelObject),
								centre( aCentrePoint),
								size( 0, 0),
								title( aTitle),
								titleSize( 0, 0),
								borderWidth( aBorderWidth),
								spacing( aSpacing)
	{
	}
	/**
	 *
	 */
	RectangleShape::RectangleShape( const wxPoint& aCentrePoint,
									const std::string& aTitle /*= ""*/,
									int aBorderWidth /*= 2*/,
									int aSpacing /*= 2*/) :
								centre( aCentrePoint),
								size( 0, 0),
								title( aTitle),
								titleSize( 0, 0),
								borderWidth( aBorderWidth),
								spacing( aSpacing)
	{
	}
	/**
	 *
	 */
	RectangleShape& RectangleShape::operator=( const RectangleShape& aRectangleShape)
	{
		if (this != &aRectangleShape)
		{
			Shape::operator=( aRectangleShape);

			centre = aRectangleShape.centre;
			size = aRectangleShape.size;
			title = aRectangleShape.title;
			titleSize = aRectangleShape.titleSize;
			borderWidth = aRectangleShape.borderWidth;
			spacing = aRectangleShape.spacing;
		}
		return *this;
	}
	/**
	 *
	 */
	void RectangleShape::draw( wxDC& dc)
	{
		// The minimum size of the RectangleShape is the size of the title
		titleSize = dc.GetTextExtent( title);
		if (size.x < (titleSize.x + 2 * spacing + 2 * borderWidth))
		{
			size.x = titleSize.x + 2 * spacing + 2 * borderWidth;
		}
		if (size.y < (titleSize.y + 2 * spacing + 2 * borderWidth))
		{
			size.y = titleSize.y + 2 * spacing + 2 * borderWidth;
		}
		// Draws a rectangle with the given top left corner, and with the given size.
		dc.SetBrush( *wxWHITE_BRUSH);
		if (isSelected())
		{
			dc.SetPen( wxPen(  getSelectionColour(), borderWidth, wxPENSTYLE_SOLID));
		} else
		{
			dc.SetPen( wxPen(  getNormalColour(), borderWidth, wxPENSTYLE_SOLID));
		}

		int x = centre.x - (size.x / 2);
		int y = centre.y - (size.y / 2);
		dc.DrawRectangle( x, y, size.x, size.y);

		dc.SetPen( wxPen(  "BLACK", borderWidth, wxPENSTYLE_SOLID));
		dc.DrawText( title, centre.x - titleSize.x / 2, y + spacing + borderWidth);
	}
	/**
	 *
	 */
	bool RectangleShape::occupies( const wxPoint& aPoint) const
	{
		if (centre.x - (size.x / 2) <= aPoint.x && aPoint.x <= centre.x + (size.x / 2) && centre.y - (size.y / 2) <= aPoint.y && aPoint.y <= centre.y + (size.y / 2))
		{
			return true;
		}
		return false;
	}
	/**
	 *
	 */
	// cppcheck-suppress unusedFunction
	bool RectangleShape::isBorderPoint( const wxPoint& aPoint,
										int aRadius /*= 3*/) const
	{
		wxPoint rectanglePoints[] = {
								   wxPoint( centre.x - (size.x / 2), centre.y - (size.y / 2)), // upper left
								   wxPoint( centre.x + (size.x / 2), centre.y - (size.y / 2)), // upper right
								   wxPoint( centre.x + (size.x / 2), centre.y + (size.y / 2)), // right under
								   wxPoint( centre.x - (size.x / 2), centre.y + (size.y / 2)), // left under
		};

		return Utils::Shape2DUtils::isOnLine( rectanglePoints, 4, aPoint, aRadius + borderWidth);
	}
	/**
	 *
	 */
	wxPoint RectangleShape::getCentre() const
	{
		return centre;
	}
	/**
	 *
	 */
	void RectangleShape::setCentre( const wxPoint& aPoint)
	{
		centre = aPoint;
	}
	/**
	 *
	 */
	std::string RectangleShape::getTitle() const
	{
		return title;
	}
	/**
	 *
	 */
	void RectangleShape::setTitle( const std::string& aTitle)
	{
		title = aTitle;
	}
	/**
	 *
	 */
wxSize RectangleShape::getSize() const
	{
		return size;
	}
	/**
	 *
	 */
	void RectangleShape::setSize( const wxSize& aSize)
	{
		size = aSize;
		if (size.x < (titleSize.x + 2 * spacing + 2 * borderWidth))
		{
			size.x = titleSize.x + 2 * spacing + 2 * borderWidth;
		}
		if (size.y < (titleSize.y + 2 * spacing + 2 * borderWidth))
		{
			size.y = titleSize.y + 2 * spacing + 2 * borderWidth;
		}
	}
	/**
	 *
	 */
	int RectangleShape::getBorderWidth() const
	{
		return borderWidth;
	}
	/**
	 *
	 */
	void RectangleShape::setBorderWidth( int aBorderWidth)
	{
		borderWidth = aBorderWidth;
	}
	/**
	 *
	 */
	int RectangleShape::getSpacing() const
	{
		return spacing;
	}
	/**
	 *
	 */
	void RectangleShape::setSpacing( int aSpacing)
	{
		spacing = aSpacing;
	}

	/**
	 *
	 */
	void RectangleShape::handleActivated()
	{
	}
	/**
	 *
	 */
	void RectangleShape::handleSelection()
	{
	}
	/**
	 *
	 */
	std::string RectangleShape::asString() const
	{
		std::ostringstream os;

		os << Shape::asString() << ", ";
		os << "centre: " << Utils::Shape2DUtils::asString( centre) << ", ";
		os << "size: " << Utils::Shape2DUtils::asString( size) << ", ";
		os << "title: " << title << ", ";
		os << "titleSize: " << Utils::Shape2DUtils::asString( titleSize) << ", ";
		os << "borderWidth: " << borderWidth << ", ";
		os << "spacing: " << spacing << ", ";
		os << "modelObject: " << (getModelObject() ? getModelObject()->asString() : "none");

		return os.str();
	}
	/**
	 *
	 */
	std::string RectangleShape::asDebugString() const
	{
		std::ostringstream os;

		os << "RectangleShape:\n";
		os << Shape::asDebugString() << "\n";
		os << "\tcentre: " << Utils::Shape2DUtils::asString( centre) << "\n";
		os << "\tsize: " << Utils::Shape2DUtils::asString( size) << "\n";
		os << "\ttitle: " << title << "\n";
		os << "\ttitleSize: " << Utils::Shape2DUtils::asString( titleSize) << "\n";
		os << "\tborderWidth: " << borderWidth << "\n";
		os << "\tspacing: " << spacing << "\n";
		os << "\tmodelObject: " << (getModelObject() ? getModelObject()->asDebugString() : "none");

		return os.str();
	}
} // namespace View
