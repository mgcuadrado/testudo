<?xml version = "1.0" encoding = "UTF-8"?>
<xsl:stylesheet version = "1.0"
                xmlns:xsl = "http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text"/>

  <xsl:template match="interactive_test">
    <xsl:text>interactive test</xsl:text>
  </xsl:template>

  <xsl:template match="testudo">
    <xsl:apply-templates select="test"/>
  </xsl:template>

  <xsl:template match="test">
    <xsl:text>||~ident~||{||~normal~||||~indent_as_b~||</xsl:text>
    <xsl:text/><xsl:value-of select="@name"/>
    <xsl:text>||~indent_as_e~||||~ident~||}||~normal~|| </xsl:text>
    <xsl:value-of select="@n_failed"/>/<xsl:value-of select="@n_total"/>
    <xsl:text> fail</xsl:text>
    <xsl:if test="@n_errors!='0'">
      <xsl:text>, </xsl:text>
      <xsl:value-of select="@n_errors"/>
      <xsl:text> err</xsl:text>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@n_errors='0'">
        <xsl:apply-templates select="@success"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="flag_error"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>&#xa;</xsl:text>

    <xsl:apply-templates select="
        test
      "/>
  </xsl:template>

  <xsl:template match="@success">
    <xsl:if test=".='true'">
      <xsl:text> ||~right~|| [||~successtag~|| OK ||~normal~||]</xsl:text>
    </xsl:if>
    <xsl:if test=".='false'">
      <xsl:text>||~failure~|| ||~right~||-||~normal~||</xsl:text>
      <xsl:text> [||~failuretag~||FAIL||~normal~||]</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="flag_error">
    <xsl:text>||~failure~|| ||~right~||-||~normal~||</xsl:text>
    <xsl:text> [||~error~||ERR-||~normal~||]</xsl:text>
  </xsl:template>

</xsl:stylesheet>
